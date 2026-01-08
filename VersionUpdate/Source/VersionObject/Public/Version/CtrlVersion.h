
#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "CtrlVersion.generated.h"

// 是为了和独立程序对应
static FString CtrlVerClientJsonRelativePath = TEXT("Version/ClientVersion.config");

UENUM(BlueprintType)
enum class EPatchFileTag : uint8
{
	None   UMETA(DisplayName = "None"),
	Pak    UMETA(DisplayName = "Pak"),
	PakSig    UMETA(DisplayName = "PakSig"),
	DLC    UMETA(DisplayName = "DLC"),
	DLL    UMETA(DisplayName = "DLL"),
	Temp   UMETA(DisplayName = "Temp"),
	Custom UMETA(DisplayName = "Custom"),
};


USTRUCT(BlueprintType)
struct VERSIONOBJECT_API FRemotePatchFile
{
	GENERATED_BODY()

	// 本地文件路径（用于生成 Size / Hash）
	//TODO:复制出来方便管理
	//TODO:Client下也考虑用这个存储路径
	UPROPERTY(EditAnywhere, Category = "Ctrl Version")
	FFilePath SourceFile;

	// patch_10100_p.pak
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	FString Name;

	// MD5 / SHA1
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	FString Hash;

	// 文件大小（字节）
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	int64 Size = 0;

	// 安装目录：Paks / Content / Custom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	FString InstallDir = TEXT("Paks");

	// 标签
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	EPatchFileTag Tag = EPatchFileTag::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	bool bDiscard = false;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	//bool bMajorVersion = false;
};

FORCEINLINE bool operator==(const FRemotePatchFile& L, const FRemotePatchFile& R)
{
	return L.Name == R.Name && L.Hash == R.Hash && L.Size ==R.Size;
}


USTRUCT(BlueprintType)
struct VERSIONOBJECT_API FPatchVersion
{
	GENERATED_BODY()

	// 1.1 / 1.2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	FString Version;

	// 该版本需要的文件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	TArray<FRemotePatchFile> Files;
};

USTRUCT(BlueprintType)
struct VERSIONOBJECT_API FPatchList
{
	GENERATED_BODY()

	// 支持的安装版本（否则强制整包）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	TArray<FString> SupportVersions;

	// url 最好由domain + "更新包桶(例如Package)" + 游戏名 + 平台 组成，最终url 类似 https://mydomain.com/Package/TestGame/Windows/Ctrl Version.pak Ctrl Version.pak是FPatchVersion 内的Files
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	FString Url;

	// 所有补丁版本
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	TArray<FPatchVersion> Patches;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ctrl Version")
	TArray<FRemotePatchFile> MajorVersionFiles;
};

namespace VersionControl
{
	VERSIONOBJECT_API void WritePatchFile(TSharedRef<TJsonWriter<>> Writer, const FRemotePatchFile& File);
	VERSIONOBJECT_API void ReadPatchFile(const TSharedPtr<FJsonObject>& Obj, FRemotePatchFile& Out);
	VERSIONOBJECT_API void Save(const FPatchList& Info, FString& OutJson);
	VERSIONOBJECT_API bool Read(const FString& Json, FPatchList& OutInfo);
}