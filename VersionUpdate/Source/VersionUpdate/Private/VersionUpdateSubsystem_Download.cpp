#include "VersionUpdateSubsystem.h"

#include "Misc/FileHelper.h"
#include "VersionUpdateLogChannels.h"

namespace
{
	// 下载保存前确保目录存在。
	void EnsureDirectory(const FString& InPath)
	{
		if (!IFileManager::Get().DirectoryExists(*InPath))
		{
			IFileManager::Get().MakeDirectory(*InPath, true);
		}
	}

	// 服务器 Manifest 只存根 URL 和文件名；下载前统一拼成完整 URL。
	FString CombineURL(const FString& BaseURL, const FString& FileName)
	{
		FString Result = BaseURL;
		Result.RemoveFromEnd(TEXT("/"));
		return Result / FileName;
	}

	// 非 Pak 文件最终要安装到 Manifest 指定目录，因此下载时提前记录目标路径。
	FString GetCustomInstallPath(const FRemotePatchFile& File, const FString& FileName)
	{
		FString InstallPath = File.InstallDir;
		FPaths::NormalizeFilename(InstallPath);
		InstallPath.RemoveFromStart(TEXT("/"));
		InstallPath.RemoveFromEnd(TEXT("/"));
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / InstallPath / FileName);
	}
}

void UVersionUpdateSubsystem::DownloadPendingFiles()
{
	// 每次下载开始时重置本轮下载状态。任何单文件失败都会让最终完成事件返回 false。
	bLastDownloadSucceeded = true;

	TArray<FString> DownloadURLs;
	for (const FRemotePatchFile& File : PendingDownloadFiles)
	{
		if (!File.Name.IsEmpty())
		{
			DownloadURLs.AddUnique(CombineURL(ServerManifest.Url, File.Name));
		}
	}

	if (DownloadURLs.Num() == 0)
	{
		// 可能只有废弃文件需要处理，或者已经是最新；仍然广播完成，让业务层流程闭环。
		OnVersionDownloadCompleted.Broadcast(true);
		return;
	}

	// 使用 HTTP 封装批量下载；所有回调只更新下载状态，不触发安装。
	VersionUpdateHTTP::FHTTPDelegate HTTPDelegate;
	HTTPDelegate.HttpCompleteDelegate.BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
	{
		OnDownloadRequestComplete(Request, Response, bSucceeded);
	});

	HTTPDelegate.HttpSingleRequestProgressDelegate.BindLambda([this](FHttpRequestPtr Request, int32 TotalBytes, int32 BytesReceived)
	{
		OnDownloadRequestProgress(Request, TotalBytes, BytesReceived);
	});

	HTTPDelegate.AllTasksCompletedDelegate.BindLambda([this]()
	{
		OnDownloadRequestsComplete();
	});

	VersionUpdateHTTP::FHTTP::CreateHTTPObject(HTTPDelegate)->GetObjectsToMemory(DownloadURLs);
}

FString UVersionUpdateSubsystem::GetDownloadPathToPak() const
{
	// Pak / PakSig 下载到 Saved/TmpPak，安装阶段再复制到 Content/Paks。
	const FString DownloadPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("TmpPak"));
	EnsureDirectory(DownloadPath);
	return DownloadPath;
}

FString UVersionUpdateSubsystem::GetDownloadPathToCustom() const
{
	// 非 Pak 文件下载到 Saved/TmpCustom，安装阶段按 RelativePatchs 映射复制到最终目录。
	const FString DownloadPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("TmpCustom"));
	EnsureDirectory(DownloadPath);
	return DownloadPath;
}

void UVersionUpdateSubsystem::OnDownloadRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	// 单文件下载完成后，根据 URL 文件名回查 PendingDownloadFiles 中的 Manifest 描述。
	const FString FileName = FPaths::GetCleanFilename(HttpRequest->GetURL());
	if (!bSucceeded || !HttpResponse.IsValid())
	{
		// 网络失败或响应无效，记录本轮下载失败并通知业务层。
		bLastDownloadSucceeded = false;
		OnPatchFileDownloadCompleted.Broadcast(FileName, false);
		return;
	}

	const FRemotePatchFile* ServerFile = PendingDownloadFiles.FindByPredicate([&](const FRemotePatchFile& File)
	{
		return File.Name == FileName;
	});

	if (!ServerFile)
	{
		// 理论上不应发生：下载 URL 来自 PendingDownloadFiles，找不到说明队列被外部修改或 Manifest 异常。
		bLastDownloadSucceeded = false;
		OnPatchFileDownloadCompleted.Broadcast(FileName, false);
		return;
	}

	FString SavePath;
	if (ServerFile->Tag == EPatchFileTag::Pak || ServerFile->Tag == EPatchFileTag::PakSig)
	{
		// Pak 类文件走 Pak 临时目录。
		SavePath = GetDownloadPathToPak() / FileName;
	}
	else
	{
		// Custom/DLL/DLC 等非 Pak 文件走 Custom 临时目录，并记录最终安装路径。
		SavePath = GetDownloadPathToCustom() / FileName;

		const FString FinalPath = GetCustomInstallPath(*ServerFile, FileName);
		EnsureDirectory(FPaths::GetPath(FinalPath));
		RelativePatchs.AddUnique(FinalPath);
	}

	EnsureDirectory(FPaths::GetPath(SavePath));

	const TArray<uint8>& Data = HttpResponse->GetContent();
	const bool bSaved = FFileHelper::SaveArrayToFile(Data, *SavePath);
	// bLastDownloadSucceeded 会保留任意一次失败，最终由 OnDownloadRequestsComplete 广播。
	bLastDownloadSucceeded &= bSaved;
	OnPatchFileDownloadCompleted.Broadcast(FileName, bSaved);
}

void UVersionUpdateSubsystem::OnDownloadRequestProgress(FHttpRequestPtr HttpRequest, int32 TotalBytes, int32 BytesReceived)
{
	// 下载进度按单文件广播；总进度由业务层根据文件数量或大小自行汇总更灵活。
	const FString FileName = FPaths::GetCleanFilename(HttpRequest->GetURL());
	const float Percentage = TotalBytes > 0 ? static_cast<float>(BytesReceived) / static_cast<float>(TotalBytes) : 0.f;
	OnVersionDownloadProgress.Broadcast(Percentage, FileName, TotalBytes, BytesReceived);
}

void UVersionUpdateSubsystem::OnDownloadRequestsComplete()
{
	// 所有下载任务完成后只广播结果，不自动安装，保持事件驱动。
	OnVersionDownloadCompleted.Broadcast(bLastDownloadSucceeded);
}
