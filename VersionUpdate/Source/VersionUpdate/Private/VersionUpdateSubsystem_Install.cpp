#include "VersionUpdateSubsystem.h"

#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "VersionInstallationManage.h"
#include "VersionPakBPLibrary.h"
#include "VersionUpdateLogChannels.h"

namespace
{
	// 安装进度由安装管理器拷贝文件时回调 Poll 累加。
	float PakNumber = 0.f;
	float CustomPakNumber = 0.f;
	float LastFraction = 0.f;
	float PercentageInterval = 0.f;
	float VersionUpdateInstallationProgressValue = 0.f;

	// 外部安装器和项目启动程序目录使用平台名；Windows 下固定为 Win64。
	FString GetPlatformName()
	{
		const FString PlatformName = FPlatformProperties::PlatformName();
		return PlatformName.Contains(TEXT("Windows")) ? TEXT("Win64") : PlatformName;
	}
	
	FString GetInstallRelativeFilePath(const FRemotePatchFile& File)
	{
		// InstalledPatchFiles 需要按最终安装位置去重，不能只比较 Name。
		FString InstallDir = File.InstallDir;
		FPaths::NormalizeFilename(InstallDir);
		InstallDir.RemoveFromStart(TEXT("/"));
		InstallDir.RemoveFromEnd(TEXT("/"));
		return InstallDir.IsEmpty() ? File.Name : InstallDir / File.Name;
	}

	// 丢弃文件需要转换成本地绝对路径传给安装器。
	FString GetInstallAbsoluteFilePath(const FRemotePatchFile& File)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / GetInstallRelativeFilePath(File));
	}



	// IFileManager::Copy 的进度回调。这里按文件数量平均累加整体安装进度。
	struct FPakInstallationProgress : public FCopyProgress
	{
		virtual ~FPakInstallationProgress() {}

		virtual bool Poll(float Fraction) override
		{
			PercentageInterval = Fraction - LastFraction;

			const float Total = PakNumber + CustomPakNumber;
			if (Total > 0.f)
			{
				VersionUpdateInstallationProgressValue += PercentageInterval / Total;
			}

			LastFraction = (Fraction == 1.f) ? 0.f : Fraction;
			return true;
		}
	};
}

float UVersionUpdateSubsystem::GetInstallationProgress() const
{
	// 返回安装阶段进度。下载进度请使用 OnVersionDownloadProgress。
	return VersionUpdateInstallationProgressValue;
}

// 每次版本检测或安装前重置，避免上一次安装进度残留。
void UVersionUpdateSubsystem::ResetInstallationProgress()
{

	PakNumber = 0.f;
	CustomPakNumber = 0.f;
	LastFraction = 0.f;
	PercentageInterval = 0.f;
	VersionUpdateInstallationProgressValue = 0.f;
}

// 安装下载文件，业务层可以在下载完后调用此函数进行安装
bool UVersionUpdateSubsystem::InstallDownloadedFiles()
{
	const bool bSucceeded = bSeamlessHotUpdate ? ExecuteSeamlessInstall() : LaunchExternalInstaller();
	OnVersionInstallCompleted.Broadcast(bSucceeded);
	return bSucceeded;
}

bool UVersionUpdateSubsystem::ExecuteSeamlessInstall()
{
	ResetInstallationProgress();

	// 无感安装会在当前进程内覆盖资源，因此先卸载已挂载 Pak，避免文件占用。
	if (!UnmountMountedPaks())
	{
		return false;
	}

	// 临时下载目录和最终安装目录。
	const FString ResourcesToCopiedCustom = GetDownloadPathToCustom();
	const FString ResourcesToCopied = GetDownloadPathToPak();
	const FString ProjectToContentPaks = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Paks"));
	const FString ProjectRootPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

	// 执行复制、删除临时文件、处理废弃文件等安装动作。
	const bool bInstallOK = FVersionInstallationManage().HandleHotInstallation<FPakInstallationProgress>(
		ResourcesToCopied,
		ResourcesToCopiedCustom,
		ProjectToContentPaks,
		ProjectRootPath,
		RelativePatchs,
		PendingDownloadFiles,
		PendingDiscardFiles,
		PakNumber,
		CustomPakNumber,
		VersionUpdateInstallationProgressValue);

	if (!bInstallOK || !RemountCachedPaks())
	{
		// 安装失败或重新挂载失败，都视为无感安装失败。
		return false;
	}

	// 安装成功后持久化客户端状态。
	// 先更新内存中的安装结果，再写 json；写入失败时仍按安装失败处理，避免本地状态失真。
	UpdateClientManifestAfterInstall();
	if (!SaveClientManifest())
	{
		return false;
	}

	const FString StartupMap = GetDefault<UGameMapsSettings>()->GetGameDefaultMap();
	if (!StartupMap.IsEmpty())
	{
		// 无感安装完成后重进启动地图，让新资源状态更稳定地生效。
		UGameplayStatics::OpenLevel(GetWorld(), *StartupMap);
	}

	return true;
}

// 当前 Manifest 没有独立的目标客户端版本字段。
// 热更新路径下，这里约定使用最后一个补丁版本作为安装完成后的本地版本号。
FString UVersionUpdateSubsystem::GetInstalledTargetVersion() const
{
	for (int32 Index = ServerManifest.Patches.Num() - 1; Index >= 0; --Index)
	{
		if (!ServerManifest.Patches[Index].Version.IsEmpty())
		{
			return ServerManifest.Patches[Index].Version;
		}
	}

	return CurrentVersion;
}

void UVersionUpdateSubsystem::UpdateClientManifestAfterInstall()
{
	// 本地 json 记录的是“已经安装到磁盘上的版本”，不是当前进程启动时的旧版本。
	ClientManifest.CurrentVersion = GetInstalledTargetVersion();

	TMap<FString, FRemotePatchFile> InstalledFilesByPath;
	for (const FRemotePatchFile& InstalledFile : ClientManifest.InstalledPatchFiles)
	{
		if (!InstalledFile.Name.IsEmpty())
		{
			InstalledFilesByPath.Add(GetInstallRelativeFilePath(InstalledFile), InstalledFile);
		}
	}

	TSet<FString> DiscardKeys;
	auto MarkDiscarded = [&DiscardKeys, &InstalledFilesByPath](const FRemotePatchFile& File)
	{
		if (File.Name.IsEmpty())
		{
			return;
		}

		const FString FileKey = GetInstallRelativeFilePath(File);
		DiscardKeys.Add(FileKey);
		InstalledFilesByPath.Remove(FileKey);
	};

	// 先收集所有应从 InstalledPatchFiles 中删除的目标路径。
	// 这里只记录路径 key，不直接删除文件；物理删除由安装器完成。
	for (const FRemotePatchFile& DiscardFile : PendingDiscardFiles)
	{
		MarkDiscarded(DiscardFile);
	}

	for (const FPatchVersion& PatchVersion : ServerManifest.Patches)
	{
		for (const FRemotePatchFile& ServerFile : PatchVersion.Files)
		{
			if (ServerFile.bDiscard)
			{
				MarkDiscarded(ServerFile);
			}
		}
	}

	auto AddInstalled = [&DiscardKeys, &InstalledFilesByPath](const FRemotePatchFile& InstalledFile)
	{
		const FString InstalledKey = GetInstallRelativeFilePath(InstalledFile);
		// 同一路径只保留一条记录；如果该路径被标记为丢弃，则不再写回 InstalledPatchFiles。
		if (InstalledFile.Name.IsEmpty() || InstalledFile.bDiscard || DiscardKeys.Contains(InstalledKey))
		{
			return;
		}

		InstalledFilesByPath.Add(InstalledKey, InstalledFile);
	};

	// 先移除所有废弃项，再重建应保留的文件列表，避免旧版本记录残留。
	// 服务端补丁列表描述热更新后应存在的完整文件集合，先按它重建基础状态。
	for (const FPatchVersion& PatchVersion : ServerManifest.Patches)
	{
		for (const FRemotePatchFile& ServerFile : PatchVersion.Files)
		{
			AddInstalled(ServerFile);
		}
	}

	// 再用本轮实际下载的文件覆盖一次，确保 Hash / Size 等元数据与本次安装目标一致。
	for (const FRemotePatchFile& InstalledFile : PendingDownloadFiles)
	{
		AddInstalled(InstalledFile);
	}

	ClientManifest.InstalledPatchFiles.Reset();
	InstalledFilesByPath.GenerateValueArray(ClientManifest.InstalledPatchFiles);
	ClientManifest.InstalledPatchFiles.Sort([](const FRemotePatchFile& Left, const FRemotePatchFile& Right)
	{
		return GetInstallRelativeFilePath(Left) < GetInstallRelativeFilePath(Right);
	});
}

bool UVersionUpdateSubsystem::LaunchExternalInstaller()
{
	// 外部安装器需要知道哪些本地文件应被删除。
	FString DiscardsPathsStr;
	for (const FRemotePatchFile& File : PendingDiscardFiles)
	{
		if (!File.Name.IsEmpty())
		{
			DiscardsPathsStr += GetInstallAbsoluteFilePath(File) + TEXT("|");
		}
	}

	DiscardsPathsStr.RemoveFromEnd(TEXT("|"));
	if (DiscardsPathsStr.IsEmpty())
	{
		// 安装器使用 NONE 表示没有废弃文件，避免空参数歧义。
		DiscardsPathsStr = TEXT("NONE");
	}

	// 安装器随引擎二进制放置，平台目录按 UE 规则解析。
	const FString ExePath = FPaths::ConvertRelativePathToFull(
		FPaths::EngineDir() / TEXT("Binaries") / GetPlatformName() / TEXT("VersionInstallationProgress.exe"));

	if (!FPaths::FileExists(ExePath))
	{
		UE_LOG(LogVersionUpdate, Error, TEXT("[VersionUpdate] Installer not found: %s"), *ExePath);
		return false;
	}

	FString Param;
	// 参数格式必须和 VersionInstallationProgress.exe 的命令行解析保持一致。
	Param += TEXT("-ResourcesToCopiedCustom=") + GetDownloadPathToCustom();
	Param += TEXT(" -ResourcesToCopied=") + GetDownloadPathToPak();
	Param += TEXT(" -ProjectToContentPaks=") + FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Paks"));
	Param += TEXT(" -ProjectStartupPath=") + FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Binaries") / GetPlatformName() / FApp::GetProjectName() + TEXT(".exe"));
	Param += TEXT(" -ProjectProcessID=") + FString::FromInt(FPlatformProcess::GetCurrentProcessId());
	Param += TEXT(" -Discards=") + DiscardsPathsStr;
	Param += TEXT(" -ProjectRootPath=") + FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	Param += TEXT(" -LinkURL=") + ServerManifest.Url;

	if (RelativePatchs.Num() > 0)
	{
		// 非 Pak 文件的“临时文件 -> 最终路径”映射，用 + 拼接传给安装器。
		FString RelativePatchParam;
		for (const FString& Path : RelativePatchs)
		{
			RelativePatchParam += FPaths::ConvertRelativePathToFull(Path) + TEXT("+");
		}
		RelativePatchParam.RemoveFromEnd(TEXT("+"));
		Param += TEXT(" -RelativePatchs=") + RelativePatchParam;
	}

	const FProcHandle Handle = FPlatformProcess::CreateProc(*ExePath, *Param, false, false, false, nullptr, 0, nullptr, nullptr);
	if (Handle.IsValid())
	{
		// 外部安装器接管后退出当前进程，避免文件占用。
		FPlatformMisc::RequestExit(true);
	}

	return Handle.IsValid();
}

bool UVersionUpdateSubsystem::UnmountMountedPaks()
{
	// 缓存当前挂载列表，安装完成后 RemountCachedPaks 会尝试恢复。
	MountedPakCache.Empty();
	UVersionPakBPLibrary::GetMountedPakFilenames(MountedPakCache);

	for (const FString& PakFile : MountedPakCache)
	{
		if (!UVersionPakBPLibrary::UnmountPak(PakFile))
		{
			// 任意 Pak 卸载失败都中断无感安装。
			return false;
		}
	}

	return true;
}

bool UVersionUpdateSubsystem::RemountCachedPaks()
{
	// 重新挂载卸载前缓存的 Pak。当前实现保持旧行为：挂载失败只记录在底层，不中断返回。
	for (const FString& PakFile : MountedPakCache)
	{
		UVersionPakBPLibrary::MountPak(PakFile, 0, TEXT(""));
	}

	return true;
}
