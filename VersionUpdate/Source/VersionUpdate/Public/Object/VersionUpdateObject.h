#pragma once

#include "CoreMinimal.h"
#include "Version/PatchManifest.h"
#include "HTTP/VersionUpdateHTTP.h"
#include "VersionUpdateType.h"
#include "Settings/VersionManifestClientObject.h"
#include "Tickable.h"
#include "VersionUpdateObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FProgressBPDelegate, float ,InPercentage, const FString& ,InContent, int32 ,TotalBytes, int32 ,BytesReceived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVersionBPDelegate, EServerVersionResponseType, ServerVerResponseType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectCompleteBPDelegate, const FString &,InName);

UCLASS(BlueprintType, Blueprintable)
class VERSIONUPDATE_API UVersionUpdateObject :public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//包的下载进度
	UPROPERTY(BlueprintAssignable)
	FProgressBPDelegate OnProgressDelegate;

	//所有包下载完毕后执行本次代理
	UPROPERTY(BlueprintAssignable)
	FVersionBPDelegate OnVersionDelegate;

	//每间隔线上服务器版本验证
	UPROPERTY(BlueprintAssignable)
	FVersionBPDelegate OnRealTimeVersionDetectionDelegate;

	//单个对象完成 //TODO 加个下载失败标记
	UPROPERTY(BlueprintAssignable)
	FObjectCompleteBPDelegate OnObjectCompleteBPDelegate;

public:
	UVersionUpdateObject();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CreateVersionObject", Keywords = "CVO"), Category = "VersionControl")
	static UVersionUpdateObject* CreateObject(UClass* InClass = NULL, UObject* InParent = NULL);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "IV"), Category = "Version")
	virtual bool InitVersion();
	
	UFUNCTION(BlueprintCallable, meta = (Keywords = "GCV"), Category = "Version")
	bool GetCurrentServerVersion(bool bSynchronous = true);
	
	UFUNCTION(BlueprintCallable, meta = (Keywords = "UV"), Category = "Version")
	void UpdateVersion();

	

	UFUNCTION(BlueprintCallable, meta = (Keywords = "RPP"), Category = "Version")
	void ResetProgramProgress();

public:
	virtual void Tick(float DeltaTime);
	virtual TStatId GetStatId() const;

protected:
	UFUNCTION(BlueprintCallable, meta = (Keywords = "GDPP"), Category = "Version")
	FString GetDownLoadPathToPak();

	UFUNCTION(BlueprintCallable, meta = (Keywords = "GDPT"), Category = "Version")
	FString GetDownLoadPathToCustom();

protected:
	void OnRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void OnRequestProgress(FHttpRequestPtr HttpRequest, int32 TotalBytes, int32 BytesReceived);
	void OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue);
	void OnRequestsComplete();

	void OnCompareServerPatchsList();
	void OnUpdateMajorVersion();
	

protected:
	UFUNCTION(BlueprintCallable, Category = "Version")
	bool CallInstallationProgress();

	//执行全卸载安装
	UFUNCTION(BlueprintCallable, Category = "Version")
	bool ExecuteInstallationProgressByUninstallAll();

	UFUNCTION(BlueprintCallable, Category = "Version")
	bool UnLoadAllPak();

	UFUNCTION(BlueprintCallable, Category = "Version")
	bool LoadAllPakCache();

public:
	UFUNCTION(BlueprintCallable, Category = "Version")
	void InitVersionControlObject();

	UFUNCTION(BlueprintCallable, Category = "Version")
	bool SerializeClientVersion();

	UFUNCTION(BlueprintCallable, Category = "Version")
	bool DeserializationClientVersion();

	UFUNCTION(BlueprintCallable, Category = "Version")
	FString GetClientVersionRelativePath();


	UFUNCTION(BlueprintPure, Category = "Version")
	float GetProgressInstallationPercentage();

protected:

	//初始化
	bool bInitialization;

	float VersionDetectionTime;

	bool bMajorVersion;

	bool bSeamlessHotUpdate;

protected:
	// 服务器Json
	FPatchList ServerVersionJson;

	//客户端列表
	FClientVersionFilesList ClientVersionJson;

	//UVersionManifestClientObject* ClientVersionManifest;

	// 待下载
	TArray<FRemotePatchFile> PendingDownloadFiles;

	// 待丢弃
	TArray<FRemotePatchFile> PendingDiscardFiles;

protected:
	TArray<FString> RelativePatchs;
	TArray<FString> PakFileCache;
};

