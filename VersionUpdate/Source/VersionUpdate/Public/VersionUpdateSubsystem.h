#pragma once

#include "CoreMinimal.h"
#include "HTTP/VersionUpdateHTTP.h"
#include "Settings/VersionManifestClientObject.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Version/PatchManifest.h"
#include "VersionUpdateType.h"
#include "VersionUpdateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVersionUpdateInitializedDelegate, const FString&, CurrentVersion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineVersionCheckRequestedDelegate, const FString&, ManifestURL);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVersionCheckCompletedDelegate, EServerVersionResponseType, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FVersionDownloadProgressDelegate, float, Percentage, const FString&, FileName, int32, TotalBytes, int32, BytesReceived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPatchFileDownloadCompletedDelegate, const FString&, FileName, bool, bSucceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVersionDownloadCompletedDelegate, bool, bSucceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVersionInstallCompletedDelegate, bool, bSucceeded);

/**
 * 事件驱动版本更新子系统。
 *
 * Subsystem 只负责流程调度和事件广播：
 * 1. InitializeVersionUpdate 设置本次运行的权威版本号，并广播 OnVersionUpdateInitialized。
 * 2. RequestOnlineVersionCheck 广播 OnOnlineVersionCheckRequested，业务层可自行下载 Manifest。
 * 3. SubmitServerManifest / SubmitServerManifestJson 注入 Manifest 并触发版本差异判断。
 * 4. DownloadPendingFiles 下载检测阶段生成的文件队列，并在完成后广播 OnVersionDownloadCompleted。
 * 5. InstallDownloadedFiles 安装已下载文件，并广播 OnVersionInstallCompleted。
 */
UCLASS(BlueprintType)
class VERSIONUPDATE_API UVersionUpdateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 流程事件1 初始化完成事件：业务层可在这里开始绑定 UI 或触发服务器版本检测
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FVersionUpdateInitializedDelegate OnVersionUpdateInitialized;

	// 流程事件2.1 请求服务器下载 Manifest
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FOnlineVersionCheckRequestedDelegate OnServerManifestRequested;

	// 流程事件2.2 版本检测完成事件：返回网络错误、已最新、热更新或大版本更新。
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FVersionCheckCompletedDelegate OnVersionCheckCompleted;

	//流程事件3辅助 下载进度事件：单个文件下载过程中持续广播。
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FVersionDownloadProgressDelegate OnVersionDownloadProgress;

	//流程事件3辅助 单个补丁文件下载完成事件：成功或失败都会广播文件名。
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FPatchFileDownloadCompletedDelegate OnPatchFileDownloadCompleted;

	// 流程事件3 所有待下载文件完成事件：业务层通常在这里决定是否调用 InstallDownloadedFiles。
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FVersionDownloadCompletedDelegate OnVersionDownloadCompleted;

	//流程事件4 安装完成事件：安装成功或失败后广播。
	UPROPERTY(BlueprintAssignable, Category = "Version|Events")
	FVersionInstallCompletedDelegate OnVersionInstallCompleted;

	
	//1. 手动初始化版本更新流程
	// 版本号可以由外部提供
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	void InitializeVersionUpdate(const FString& InCurrentVersion);

	// 请求在线版本检测。默认只广播 OnServerManifestRequested，由业务层自行下载 Manifest。
	// bUseInternalHttp 为 true 时，子系统使用内置 HTTP 下载配置中的 ServerJsonURL。
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	bool RequestServerVersionCheck(bool bUseInternalHttp = false, bool bSynchronous = true);

	// 外部已经拿到 FPatchList 时调用。bEvaluateImmediately 为 true 时会立即进行版本差异判断。
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	EServerVersionResponseType SubmitServerManifest(const FPatchList& InServerManifest, bool bEvaluateImmediately = true);

	// 外部已经拿到 Manifest JSON 字符串时调用。通常用于接入其他 HTTP 插件或平台 SDK。
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	EServerVersionResponseType SubmitServerManifestJson(const FString& InServerManifestJson, bool bEvaluateImmediately = true);

	// 使用当前 ServerManifest 和 CurrentVersion 进行差异判断，并广播 OnVersionCheckCompleted。
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	EServerVersionResponseType EvaluateServerManifest();

	// 下载版本检测阶段生成的 PendingDownloadFiles。
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	void DownloadPendingFiles();

	// 安装已下载文件。业务层一般在 OnVersionDownloadCompleted 成功后调用。
	UFUNCTION(BlueprintCallable, Category = "Version|Flow")
	bool InstallDownloadedFiles();

	// 修改当前权威版本号。一般只应由 InitializeVersionUpdate 调用。
	UFUNCTION(BlueprintCallable, Category = "Version|State")
	void SetCurrentVersion(const FString& InCurrentVersion);

	// 获取当前权威版本号。
	UFUNCTION(BlueprintPure, Category = "Version|State")
	FString GetCurrentVersion() const;

	// 是否已经完成 InitializeVersionUpdate。
	UFUNCTION(BlueprintPure, Category = "Version|State")
	bool IsVersionUpdateInitialized() const;

	// 获取安装阶段进度，下载进度请使用 OnVersionDownloadProgress。
	UFUNCTION(BlueprintPure, Category = "Version|State")
	float GetInstallationProgress() const;

	// 重置安装阶段进度。通常由内部流程调用。
	UFUNCTION(BlueprintCallable, Category = "Version|State")
	void ResetInstallationProgress();

	// 保存本地 ClientManifest。通常由安装完成后内部调用。
	UFUNCTION(BlueprintCallable, Category = "Version|Client Manifest")
	bool SaveClientManifest();

	// 读取本地 ClientManifest。通常由初始化流程调用。
	UFUNCTION(BlueprintCallable, Category = "Version|Client Manifest")
	bool LoadClientManifest();

	// 获取本地 ClientManifest 路径，主要用于调试和工具。
	UFUNCTION(BlueprintCallable, Category = "Version|Client Manifest")
	FString GetClientManifestPath() const;

private:
	// 使用插件内置 HTTP 下载服务器 Manifest。外部下载方式不走这里。
	bool RequestServerManifestByInternalHttp(bool bSynchronous);

	// 普通热更新文件差异比较，生成下载和废弃队列。
	EServerVersionResponseType ComparePatchFiles();

	// 大版本更新准备，只使用服务器 MajorVersionFiles。
	EServerVersionResponseType PrepareMajorVersionUpdate();

	// 下载临时目录。
	FString GetDownloadPathToPak() const;
	FString GetDownloadPathToCustom() const;

	// 内置批量下载回调。
	void OnDownloadRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void OnDownloadRequestProgress(FHttpRequestPtr HttpRequest, int32 TotalBytes, int32 BytesReceived);
	void OnDownloadRequestsComplete();

	// 安装实现：无感安装或外部安装器。
	bool ExecuteSeamlessInstall();
	bool LaunchExternalInstaller();
	bool UnmountMountedPaks();
	bool RemountCachedPaks();

	// 初始化本地 ClientManifest，不存在时创建。
	void LoadOrCreateClientManifest();

protected:
	// 本次运行的权威版本号，不信任本地 JSON 的版本字段。
	UPROPERTY()
	FString CurrentVersion;
	
	
private:
	// 运行期状态。
	bool bVersionUpdateInitialized = false;
	bool bSeamlessHotUpdate = false;
	bool bLastDownloadSucceeded = true;

	// Manifest 和阶段性队列。
	FPatchList ServerManifest;
	FClientVersionFilesList ClientManifest;

	TArray<FRemotePatchFile> PendingDownloadFiles;
	TArray<FRemotePatchFile> PendingDiscardFiles;
	TArray<FString> RelativePatchs;
	TArray<FString> MountedPakCache;
};
