// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Version/CtrlVersion.h"
#include "VersionManifestClientObject.generated.h"


USTRUCT(BlueprintType)
struct VERSIONUPDATE_API FClientVersionFilesList
{
	GENERATED_BODY()

	/** 当前客户端版本号（如 1.0 / 1.1） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Version")
	FString CurrentVersion;

	/** 已下载的 Patch 文件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Version")
	TArray<FRemotePatchFile> InstalledPatchFiles;
};

namespace VersionClientJson {
	VERSIONUPDATE_API bool Save(const FClientVersionFilesList& Info, FString& OutJson);
	VERSIONUPDATE_API bool Read(const FString& Json, FClientVersionFilesList& OutInfo);
};

/**
 * 
 */
UCLASS()
class VERSIONUPDATE_API UVersionManifestClientObject : public UObject
{
	GENERATED_BODY()


public:

	FClientVersionFilesList ClientVersionFilesList;

	//Json Url
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Version")
	FString ServerJsonURL = "http://127.0.0.1:81/PatchList.json";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Version")
	bool bSeamlessHotUpdate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Version")
	bool bSynchronous = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Version")
	FString ClientJsonSavePath = CtrlVerClientJsonRelativePath;

};


