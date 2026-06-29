﻿

#include "Object/VersionUpdateObject.h"
#include "Log/VersionUpdateLog.h"
#include "Misc/FileHelper.h"
#include "HttpManager.h"
#include "VersionPakBPLibrary.h"
#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "VersionInstallationManage.h"
#include "VersionUpdateBPLibrary.h"

#if PLATFORM_WINDOWS
#pragma optimize("",off) 
#endif

float PakNumber = 0.f;
float CustomPakNumber = 0.f;
float LastFraction = 0.f;
float PercentageInterval = 0.f;
float ProgressInstallationPercentage = 0.f;

void UVersionUpdateObject::ResetProgramProgress()
{
	PakNumber = 0.f;
	CustomPakNumber = 0.f;
	LastFraction = 0.f;
	PercentageInterval = 0.f;
	ProgressInstallationPercentage = 0.f;
}

//安装进度
struct FPakInstallationProgress :public FCopyProgress
{
	FPakInstallationProgress(){}
	virtual ~FPakInstallationProgress() {}

	virtual bool Poll(float Fraction)
	{
		PercentageInterval = Fraction - LastFraction;

		const float Total = PakNumber + CustomPakNumber;
		if (Total > 0.f)
		{
			ProgressInstallationPercentage += PercentageInterval / Total;
		}

		if (Fraction == 1.f)
		{
			LastFraction = 0.f;
		}
		else
		{
			LastFraction = Fraction;
		}

		return true;
	}
};

UVersionUpdateObject::UVersionUpdateObject()
{
	bInitialization = false;
	bMajorVersion = false;
	bSeamlessHotUpdate = GetDefault<UVersionManifestClientObject>()->bSeamlessHotUpdate;
}

UVersionUpdateObject* UVersionUpdateObject::CreateObject(UClass* InClass, UObject* InParent)
{
	if (InParent)
	{
		if (UVersionUpdateObject* Obj = NewObject<UVersionUpdateObject>(InParent, InClass))
		{
			UE_LOG(LogVersionControl, Log, TEXT("Create an object with a parent."));
			Obj->InitVersionControlObject();
			return Obj;
		}
	}
	else
	{
		if (UVersionUpdateObject* Obj = NewObject<UVersionUpdateObject>(NULL, InClass))
		{
			Obj->AddToRoot();
			Obj->InitVersionControlObject();

			UE_LOG(LogVersionControl, Log, TEXT("Create an object with a parent."));
			return Obj;
		}
	}

	UE_LOG(LogVersionControl, Error, TEXT("Creation failed for unknown reason."));

	return nullptr;
}

//入口 一
bool UVersionUpdateObject::InitVersion()
{
	if (GEngine && true)
	{
		const FString Msg = FString::Printf(TEXT("版本更新！！！"));
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, Msg);
	}

	if (!bInitialization)
	{
		//重置程序进度
		ResetProgramProgress();

		if (GetCurrentServerVersion(GetDefault<UVersionManifestClientObject>()->bSynchronous))
		{
		}

		bInitialization = true;
		return true;
	}

	return false;
}

// 如果是在Tick检测版本更新要设置为true
bool UVersionUpdateObject::GetCurrentServerVersion(bool bSynchronous)
{
	bool bConConnectNet = false;

	FString ServerVersionConfigURL = GetDefault<UVersionManifestClientObject>()->ServerJsonURL;


	UE_LOG(LogVersionControl, Display, TEXT("ServerVersionConfigURL=%s"),*ServerVersionConfigURL);

	VersionUpdateHTTP::FHTTPDelegate Delegate;
	//TODO:整理，改成判断版本函数,不要用匿名函数
	Delegate.HttpCompleteDelegate.BindLambda([&, bSynchronous](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConConnect)
	{
		if (bConConnect)
		{
			// ------------------ 处理字符串读取问题
			TArray<uint8> Raw = Response->GetContent();

			if (Raw.Num() >= 3 &&
				Raw[0] == 0xEF &&
				Raw[1] == 0xBB &&
				Raw[2] == 0xBF)
			{
				Raw.RemoveAt(0, 3);
			}

			FUTF8ToTCHAR Conv(
				reinterpret_cast<const ANSICHAR*>(Raw.GetData()),
				Raw.Num()
			);

			FString JsonStr(Conv.Length(), Conv.Get());
			JsonStr.TrimStartInline();
			// ------------------ 处理字符串读取问题

			
			if (!VersionControl::Read(JsonStr, ServerVersionJson))
			{
				UE_LOG(LogVersionControl, Error, TEXT("The request for object server version information failed. Please check whether the permissions of the following OSS servers are public read."));
				OnRealTimeVersionDetectionDelegate.Broadcast(EServerVersionResponseType::CONNECTION_ERROR);
			}else // HTTP成功时
			{
				// TODO:大版本更新时顺便更新小版本？这里还是要把两个函数合并
				if (ServerVersionJson.SupportVersions.Contains(ClientVersionJson.CurrentVersion))
				{
					bMajorVersion = false;
					OnCompareServerPatchsList();
				}else{
					bMajorVersion = true;
					bSeamlessHotUpdate = false;
					OnUpdateMajorVersion();
				}	
				
			}
				
		}

		bConConnectNet = true;
	});

	// 二
	//访问服务器获取配置文件，成功会触发上面的绑定
	bool bCreateHTTPObject = VersionUpdateHTTP::FHTTP::CreateHTTPObject(Delegate)->GetObjectToMemory(ServerVersionConfigURL);

	if (bSynchronous)
	{
		FHttpModule::Get().GetHttpManager().Flush(EHttpFlushReason::Default);
	}
	else
	{
		bConConnectNet = bCreateHTTPObject;
	}

	return bConConnectNet;
}

//热更新
void UVersionUpdateObject::UpdateVersion()
{
	auto CombineURL = [](const FString& Base, const FString& FileName) -> FString
		{
			FString Result = Base;
			Result.RemoveFromEnd(TEXT("/"));
			return Result / FileName;
		};

	// 1️⃣ 收集所有需要下载的 URL

	TArray<FString> DownloadURLs;

	for (const FRemotePatchFile& File : PendingDownloadFiles)
	{
		DownloadURLs.AddUnique(
			CombineURL(ServerVersionJson.Url, File.Name)
		);

		UE_LOG(LogVersionControl, Log,
			TEXT("[VersionUpdate] Need download: %s"),
			*File.Name);
	}

	// 2️⃣ 没有文件需要下载
	if (DownloadURLs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VersionUpdate] No patch files found."));
		OnRequestsComplete();   
		return;
	}

	// 3️⃣ 绑定 HTTP 回调
	//TODO:改为能分片、断点续传的
	VersionUpdateHTTP::FHTTPDelegate HTTPDelegate;

	HTTPDelegate.HttpCompleteDelegate.BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
		{
			OnRequestComplete(Request, Response, bSucceeded);
		}
	);

	HTTPDelegate.HttpSingleRequestHeaderReceivedDelegate.BindLambda(
		[this](FHttpRequestPtr Request, const FString& HeaderName, const FString& HeaderValue)
		{
			OnRequestHeaderReceived(Request, HeaderName, HeaderValue);
		}
	);

	HTTPDelegate.HttpSingleRequestProgressDelegate.BindLambda(
		[this](FHttpRequestPtr Request, int32 TotalBytes, int32 BytesReceived)
		{
			OnRequestProgress(Request, TotalBytes, BytesReceived);
		}
	);

	HTTPDelegate.AllTasksCompletedDelegate.BindLambda(
		[this]()
		{
			OnRequestsComplete();
		}
	);

	// 4️⃣ 发起批量下载
	VersionUpdateHTTP::FHTTP* Http = VersionUpdateHTTP::FHTTP::CreateHTTPObject(HTTPDelegate);
	Http->GetObjectsToMemory(DownloadURLs);
}

void UVersionUpdateObject::OnUpdateMajorVersion()
{
	PendingDownloadFiles.Empty();
	PendingDiscardFiles.Empty();
	RelativePatchs.Empty();

	// ---------- 强制加入 MajorVersionFiles ----------
	for (const FRemotePatchFile& File : ServerVersionJson.MajorVersionFiles)
	{
		if (File.Name.IsEmpty())
		{
			continue;
		}

		PendingDownloadFiles.Add(File);

		UE_LOG(LogVersionControl, Warning,TEXT("[MajorUpdate][Download] %s"), *File.Name);
	}

	if (PendingDownloadFiles.Num() == 0)
	{
		UE_LOG(LogVersionControl, Error,
			TEXT("[MajorUpdate] No MajorVersionFiles found in server manifest."));

		OnRealTimeVersionDetectionDelegate.Broadcast(
			EServerVersionResponseType::CONNECTION_ERROR);
		return;
	}

	// ---------- 重置进度 ----------
	ResetProgramProgress();

	// ---------- 通知外部（UI / BP） ----------
	OnRealTimeVersionDetectionDelegate.Broadcast(
		EServerVersionResponseType::MAJORVERSION_UPDATE);
		
}

void ExistsAndCreate(const FString &InNewPath)
{
	if (!IFileManager::Get().DirectoryExists(*InNewPath))
	{
		IFileManager::Get().MakeDirectory(*InNewPath, true);
	}
}

FString UVersionUpdateObject::GetDownLoadPathToPak()
{
	FString DownloadPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("TmpPak"));
	ExistsAndCreate(DownloadPath);

	return DownloadPath;
}

FString UVersionUpdateObject::GetDownLoadPathToCustom()
{
	FString DownloadPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("TmpCustom"));
	ExistsAndCreate(DownloadPath);
	return DownloadPath;
}

//GetObjectsToMemory 回调
void UVersionUpdateObject::OnRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	const FString HeadName = FPaths::GetCleanFilename(HttpRequest->GetURL());

	// ---------- 基础校验 ----------
	if (!bSucceeded || !HttpResponse.IsValid())
	{
		UE_LOG(LogVersionControl, Error,
			TEXT("[VersionUpdate] Failed to download [%s]"),
			*HeadName);

		OnObjectCompleteBPDelegate.Broadcast(HeadName);
		return;
	}

	// ---------- 查找 Server 描述 ----------
	const FRemotePatchFile* ServerFile = nullptr;

	if (bMajorVersion)
	{
		ServerFile = ServerVersionJson.MajorVersionFiles.FindByPredicate(
			[&](const FRemotePatchFile& InFile)
			{
				return InFile.Name == HeadName;
			});
	}else
	{
		for (const FPatchVersion& PatchVersion : ServerVersionJson.Patches)
		{
			ServerFile = PatchVersion.Files.FindByPredicate(
				[&](const FRemotePatchFile& InFile)
				{
					return InFile.Name == HeadName;
				});

			if (ServerFile)
			{
				break;
			}
		}
	}


	if (!ServerFile)
	{
		UE_LOG(LogVersionControl, Error,
			TEXT("[VersionUpdate] No server file info found for [%s]"),
			*HeadName);

		OnObjectCompleteBPDelegate.Broadcast(HeadName);
		return;
	}

	// ---------- 决定保存路径 ----------
	FString SavePath;

	
	if (ServerFile->Tag == EPatchFileTag::Pak || ServerFile->Tag == EPatchFileTag::PakSig)
	{ //content/pak路径
		SavePath = GetDownLoadPathToPak() / HeadName;
	}
	else	
	{ //其他路径
		SavePath = GetDownLoadPathToCustom() / HeadName;

		// 为独立安装程序收集最终目标路径
		{
			FString InstallPath = ServerFile->InstallDir;
			FPaths::NormalizeFilename(InstallPath);

			InstallPath.RemoveFromStart(TEXT("/"));
			InstallPath.RemoveFromEnd(TEXT("/"));

			const FString FinalPath =
				FPaths::ConvertRelativePathToFull(
					FPaths::ProjectDir() / InstallPath / HeadName);
			UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] FinalPath=%s"), *FinalPath);
			ExistsAndCreate(FPaths::GetPath(FinalPath));
			RelativePatchs.AddUnique(FinalPath); //Mark:需要检查一下绝对路径问题
		}
	}
	ExistsAndCreate(FPaths::GetPath(SavePath));

	// ---------- 保存文件（整块写入，当前实现） ----------
	//TODO 保存前/后进行md5和size检验
	const TArray<uint8>& Data = HttpResponse->GetContent();

	if (!FFileHelper::SaveArrayToFile(Data, *SavePath))
	{
		UE_LOG(LogVersionControl, Error,TEXT("[VersionUpdate] Failed to save [%s] => %s"),*HeadName,*SavePath);
		OnObjectCompleteBPDelegate.Broadcast(HeadName);
		return;
	}

	UE_LOG(LogVersionControl, Log,TEXT("[VersionUpdate] Saved [%s] => %s"),*HeadName,*SavePath);


	// ---------- 通知 ----------
	OnObjectCompleteBPDelegate.Broadcast(HeadName);
}

void UVersionUpdateObject::OnRequestProgress(FHttpRequestPtr HttpRequest, int32 TotalBytes, int32 BytesReceived)
{
	FString Head = FPaths::GetCleanFilename(HttpRequest->GetURL());

	float InValue = ((float)BytesReceived / (float)TotalBytes);
	UE_LOG(LogVersionControl, Log, TEXT("[%s] : %.2lf %%"),*Head, InValue * 100.f);

	OnProgressDelegate.Broadcast(InValue, Head, TotalBytes, BytesReceived);
}

void UVersionUpdateObject::OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue)
{

}

// 只更新了版本(例如丢弃了某个包)或全部下载完成后 回调
void UVersionUpdateObject::OnRequestsComplete()
{
	UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] All download tasks completed."));

	// 是否启用无感热更新（Seamless）

	if (bSeamlessHotUpdate) // TODO: unmount会报错
	{
		// ---------- 无感热更新（当前进程内安装） ----------
		if (ExecuteInstallationProgressByUninstallAll())
		{
			UE_LOG(LogVersionControl, Log,
				TEXT("[VersionUpdate] Seamless installation success."));

			// 同步 Client Manifest / Version
			//TODO:赋值给 ClientVersionJson.CurrentVersion
			SerializeClientVersion();

			// 跳转启动地图
			const FString StartupMap =
				GetDefault<UGameMapsSettings>()->GetGameDefaultMap();

			if (StartupMap.IsEmpty())
			{
				UE_LOG(LogVersionControl, Error,
					TEXT("[VersionUpdate] Startup map is empty."));

				ensure(0);
				FPlatformMisc::RequestExit(true);
				return;
			}

			UE_LOG(LogVersionControl, Display,
				TEXT("[VersionUpdate] OpenLevel: %s"), *StartupMap);

			UGameplayStatics::OpenLevel(GetWorld(), *StartupMap);
		}
		else
		{
			UE_LOG(LogVersionControl, Error,
				TEXT("[VersionUpdate] Seamless installation failed (Major version?)"));

			ensure(0);
			FPlatformMisc::RequestExit(true);
		}
	}
	else
	{
		// ---------- 独立安装程序 ----------
		if (CallInstallationProgress())
		{
			UE_LOG(LogVersionControl, Log,
				TEXT("[VersionUpdate] External installer launched successfully."));
			
			//TODO:在独立程序里，当安装完后，更新ClientJson 中的版本号
			//SerializeClientVersion();

			// 这里通常不需要 OpenLevel
			// 等外部程序替换完 pak 后重启
		}
		else
		{
			UE_LOG(LogVersionControl, Error,
				TEXT("[VersionUpdate] Failed to launch installation process."));

			ensure(0);
		}

		// 强制退出，交给安装器
		FPlatformMisc::RequestExit(true);
	}

}

//异步版本检测
// 三
void UVersionUpdateObject::OnCompareServerPatchsList()
{
	UE_LOG(LogVersionControl, Log,
		TEXT("[VersionUpdate] Compare server version with local files (Server-only mode)"));

	PendingDownloadFiles.Empty();
	PendingDiscardFiles.Empty();

	// ---------- 遍历 ServerJson ----------
	for (const FPatchVersion& PatchVersion : ServerVersionJson.Patches)
	{
		for (const FRemotePatchFile& ServerFile : PatchVersion.Files)
		{
			if (ServerFile.Name.IsEmpty())
			{
				continue;
			}

			// ---------- 计算本地路径 ----------
			FString LocalPath;
			
			LocalPath = FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / ServerFile.InstallDir / ServerFile.Name);

			const bool bLocalExists = FPaths::FileExists(LocalPath);
			const int64 LocalSize =
				bLocalExists ? IFileManager::Get().FileSize(*LocalPath) : INDEX_NONE;
			const FString LocalHash = bLocalExists ? LexToString(FMD5Hash::HashFile(*LocalPath)):"";

			// 丢弃：Server 标记 bDiscard 且本地存在
			if (ServerFile.bDiscard)
			{
				if (bLocalExists)
				{
					PendingDiscardFiles.Add(ServerFile);

					UE_LOG(LogVersionControl, Log,TEXT("[VersionDiff][Discard] %s"),*LocalPath);
				}
				continue;
			}

			// 下载：不存在 或 Size 不一致
			if (!bLocalExists)
			{
				PendingDownloadFiles.Add(ServerFile);

				UE_LOG(LogVersionControl, Log,TEXT("[VersionDiff][Download][Missing] %s"),*LocalPath);
			}
			else if (LocalSize != ServerFile.Size || LocalHash != ServerFile.Hash)
			{
				PendingDownloadFiles.Add(ServerFile);

				UE_LOG(LogVersionControl, Log,TEXT("[VersionDiff][Download][SizeMismatch] %s (Local=%lld Server=%lld)"),*LocalPath,LocalSize,ServerFile.Size);
			}
			else
			{
				UE_LOG(LogVersionControl, Verbose,TEXT("[VersionCheck][Reuse] %s"),*LocalPath);
			}
		}
	}

	// ---------- 最终判定 ----------
	const bool bHasDiff =
		(PendingDownloadFiles.Num() > 0) || (PendingDiscardFiles.Num() > 0);

	if (bHasDiff)
	{
		UE_LOG(LogVersionControl, Log,
			TEXT("[VersionUpdate] Version diff detected. Download=%d Discard=%d"),
			PendingDownloadFiles.Num(),
			PendingDiscardFiles.Num());

		ResetProgramProgress();

		OnRealTimeVersionDetectionDelegate.Broadcast(
			EServerVersionResponseType::VERSION_HOTUPDATE);
	}
	else
	{
		UE_LOG(LogVersionControl, Log,
			TEXT("[VersionUpdate] Client is up-to-date."));

		OnRealTimeVersionDetectionDelegate.Broadcast(
			EServerVersionResponseType::VERSION_EQUAL);
	}
}

// 无感更新
bool UVersionUpdateObject::ExecuteInstallationProgressByUninstallAll()
{
	UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] ExecuteInstallationProgressByUninstallAll"));

	// 1️⃣ 卸载当前已加载的 Pak
	if (!UnLoadAllPak())
	{
		UE_LOG(LogVersionControl, Error, TEXT("[VersionUpdate] UnLoadAllPak failed."));
		return false;
	}

	// 2️⃣ 路径准备
	const FString ResourcesToCopiedCustom = GetDownLoadPathToCustom();
	const FString ResourcesToCopied = GetDownLoadPathToPak();
	const FString ProjectToContentPaks = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Paks"));
	const FString ProjectRootPath =
		FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

	// 3️⃣ 统计 Pak 数量（用于进度计算）
	PakNumber = 0.f;
	CustomPakNumber = 0.f;

	for (const FRemotePatchFile& File : PendingDownloadFiles)
	{
		if (File.Tag == EPatchFileTag::Pak || File.Tag == EPatchFileTag::PakSig)
		{
			++PakNumber;
		}
		else
		{
			++CustomPakNumber;
		}
	}

	UE_LOG(LogVersionControl, Log,TEXT("[VersionUpdate] PakNumber=%f CustomPakNumber=%f"),PakNumber, CustomPakNumber);

	// 构建 DiscardInfo
	TArray<FRemotePatchFile> DiscardInfo = PendingDiscardFiles;


	// 5️⃣ 执行安装（拷贝 / 覆盖）
	const bool bInstallOK =
		FVersionInstallationManage()
		.HandleHotInstallation<FPakInstallationProgress>(
			ResourcesToCopied,          // 主 Patch 目录
			ResourcesToCopiedCustom,    // 临时 / Custom Patch
			ProjectToContentPaks,       // Content/Paks
			ProjectRootPath,            // Project Root
			RelativePatchs,             // 最终目标路径列表
			DiscardInfo,                // 丢弃文件
			PakNumber,
			CustomPakNumber,
			ProgressInstallationPercentage
		);

	if (!bInstallOK)
	{
		UE_LOG(LogVersionControl, Error, TEXT("[VersionUpdate] HandleHotInstallation failed."));
		return false;
	}

	// 6️⃣ 重新加载 Pak ，无感更新需要
	if (!LoadAllPakCache())
	{
		UE_LOG(LogVersionControl, Error, TEXT("[VersionUpdate] LoadAllPakCache failed."));
		return false;
	}

	UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] Seamless installation completed."));
	return true;
}

bool UVersionUpdateObject::UnLoadAllPak()
{
	PakFileCache.Empty();
	UVersionPakBPLibrary::GetMountedPakFilenames(PakFileCache);

	for (auto& Tmp : PakFileCache)
	{
		if (!UVersionPakBPLibrary::UnmountPak(Tmp))
		{
			UE_LOG(LogVersionControl, Error, TEXT("[VersionUpdate] Failed to uninstall package .%s"), *Tmp);
			return false;
		}
		else
		{
			UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] Successfully uninstalled package. %s"), *Tmp);
		}
	}
	return true;
}

bool UVersionUpdateObject::LoadAllPakCache()
{
	if (PakFileCache.Num() > 0)
	{
		for (auto& Tmp : PakFileCache)
		{
			if (!UVersionPakBPLibrary::MountPak(Tmp, 0, TEXT("")))
			{
				UE_LOG(LogVersionControl, Error, TEXT("Installation failed It may have been uninstalled.%s"), *Tmp);
			}
			else
			{
				UE_LOG(LogVersionControl, Log, TEXT("Successfully installed package. %s"), *Tmp);
			}
		}
	}
	return true;
}


// 唤醒独立程序
bool UVersionUpdateObject::CallInstallationProgress()
{
	auto GetPlatformName = []() -> FString
		{
			FString PlatformName = FPlatformProperties::PlatformName();
			if (PlatformName.Contains(TEXT("Windows")))
			{
				return TEXT("Win64");
			}
			return PlatformName;
		};
	FString DiscardsPathsStr;

	for (const FRemotePatchFile& File : PendingDiscardFiles)
	{
		if (File.Name.IsEmpty())
		{
			continue;
		}

		const FString DiscardsAbsolutePath =
			FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / File.InstallDir / File.Name);
		DiscardsPathsStr += DiscardsAbsolutePath + TEXT("|");

		UE_LOG(LogVersionControl, Log,
			TEXT("[Discard] %s"), *DiscardsAbsolutePath);
	}

	DiscardsPathsStr.RemoveFromEnd(TEXT("|"));

	if (DiscardsPathsStr.IsEmpty())
	{
		DiscardsPathsStr = TEXT("NONE");
	}

	// -------- 2. 安装器 exe 路径 --------
	const FString ExePath =
		FPaths::ConvertRelativePathToFull(
			FPaths::EngineDir()
			/ TEXT("Binaries")
			/ GetPlatformName()
			/ TEXT("VersionInstallationProgress.exe")
		);

	if (!FPaths::FileExists(ExePath))
	{
		UE_LOG(LogVersionControl, Error, TEXT("Installer not found: %s"), *ExePath);
		return false;
	}

	// -------- 3. 组装命令行参数 --------
	FString Param;

	Param += TEXT("-ResourcesToCopiedCustom=") + GetDownLoadPathToCustom();
	Param += TEXT(" -ResourcesToCopied=") + GetDownLoadPathToPak();
	Param += TEXT(" -ProjectToContentPaks=")
		+ FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Paks"));

	Param += TEXT(" -ProjectStartupPath=")
		+ FPaths::ConvertRelativePathToFull(
			FPaths::ProjectDir()
			/ TEXT("Binaries")
			/ GetPlatformName()
			/ FApp::GetProjectName() + TEXT(".exe"));

	Param += TEXT(" -ProjectProcessID=")
		+ FString::FromInt(FPlatformProcess::GetCurrentProcessId());

	Param += TEXT(" -Discards=") + DiscardsPathsStr; //Mark // 传的是路径

	Param += TEXT(" -ProjectRootPath=")
		+ FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

	Param += TEXT(" -LinkURL=") + ServerVersionJson.Url;

	// -------- 4. RelativePatchs（临时包 → 最终路径）--------
	if (RelativePatchs.Num() > 0)
	{
		FString RelativePatchParam;
		for (const FString& Path : RelativePatchs)
		{
			RelativePatchParam += FPaths::ConvertRelativePathToFull(Path) + TEXT("+");
		}
		RelativePatchParam.RemoveFromEnd(TEXT("+"));

		Param += TEXT(" -RelativePatchs=") + RelativePatchParam; //Mark
	}

	// -------- 5. 启动外部安装器 --------
	FProcHandle Handle = FPlatformProcess::CreateProc(
		*ExePath,
		*Param,
		false,   // bLaunchDetached
		false,   // bLaunchHidden
		false,   // bLaunchReallyHidden
		nullptr,
		0,
		nullptr,
		nullptr
	);

	UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] [InstallerExe] %s"), *ExePath);
	UE_LOG(LogVersionControl, Log, TEXT("[VersionUpdate] [InstallerParam] %s"), *Param);

	return Handle.IsValid();
}

//初始化读或写json
void UVersionUpdateObject::InitVersionControlObject()
{

	FString ClientVersionsPaths = FPaths::ConvertRelativePathToFull(GetClientVersionRelativePath());
	if (!IFileManager::Get().FileExists(*ClientVersionsPaths))
	{
		ClientVersionJson.CurrentVersion = TEXT("-1.0.0");
		ClientVersionJson.InstalledPatchFiles.Empty();
		SerializeClientVersion();
	}
	else
	{
		DeserializationClientVersion();
	}
}
// 写client json
bool UVersionUpdateObject::SerializeClientVersion()
{
	FString Json;
	FString FilePath = GetClientVersionRelativePath();

	if (!VersionClientJson::Save(ClientVersionJson, Json))
	{
		UE_LOG(LogTemp, Error, TEXT("ImportJson failed: parse json failed %s"), *FilePath);
		return false;
	}

	if (!FFileHelper::SaveStringToFile(Json, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("SerializeClientVersion failed: save file %s"), *FilePath);
		return false;
	}

	return true;
}

// 读client json
bool UVersionUpdateObject::DeserializationClientVersion()
{
	FString FilePath = GetClientVersionRelativePath();

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("ImportJson failed: load json failed %s"), *FilePath);
		return false;
	}

	if (!VersionClientJson::Read(Json, ClientVersionJson))
	{
		UE_LOG(LogTemp, Error, TEXT("ImportJson failed: parse json failed %s"), *FilePath);
		return false;
	}

	return true;
	
}

//client json目录
FString UVersionUpdateObject::GetClientVersionRelativePath()
{
	return FPaths::ProjectContentDir() / GetDefault<UVersionManifestClientObject>()->ClientJsonSavePath;
}


float UVersionUpdateObject::GetProgressInstallationPercentage()
{
	return ProgressInstallationPercentage;
}

void UVersionUpdateObject::Tick(float DeltaTime)
{
}

TStatId UVersionUpdateObject::GetStatId() const
{
	return TStatId();
}

#if PLATFORM_WINDOWS
#pragma optimize("",on) 
#endif