

#include "VersionInstallationManage.h"
#include "Version/PatchManifest.h"

#define LOCTEXT_NAMESPACE "FVersionInstallationManage"

FVersionInstallationManage::FVersionInstallationManage()
{
}

FVersionInstallationManage::~FVersionInstallationManage()
{
}

void FVersionInstallationManage::CollectFiles(
	const FString& InResourcesToCopied,
	TArray<FString>& OutFoundFiles)
{
	//资源遍历
	UE_LOG(LogVersionInstallation, Display, TEXT("Resource traversal."));

	if (!InResourcesToCopied.IsEmpty())
	{
		IFileManager::Get().FindFilesRecursive(OutFoundFiles, *InResourcesToCopied, TEXT("*"), true, false);
	}
}

void FVersionInstallationManage::CollectFilesCustom(
	const FString& InResourcesToCopiedCustom,
	const TArray<FString>& InRelativePatchs,
	TArray<FString>& OutCustomFoundFiles,
	TArray<FVersionInstallationCustomCopyPath>& OutCustomCopyPaths)
{
	UE_LOG(LogVersionInstallation, Display, TEXT("Resource Custom traversal."));

	//收集自定义拷贝路径
	if (InRelativePatchs.Num() > 0)
	{
		IFileManager::Get().FindFilesRecursive(OutCustomFoundFiles, *InResourcesToCopiedCustom, TEXT("*"), true, false);

		for (auto& Tmp : InRelativePatchs)
		{
			FString RelativePatchKey = FPaths::GetCleanFilename(Tmp);
			for (auto& CustomTmp : OutCustomFoundFiles)
			{
				FString CustomFoundFilesKey = FPaths::GetCleanFilename(CustomTmp);
				if (RelativePatchKey == CustomFoundFilesKey)
				{
					FVersionInstallationCustomCopyPath& CopyPath = OutCustomCopyPaths.AddDefaulted_GetRef();
					CopyPath.From = CustomTmp;
					CopyPath.To = Tmp;
					break;
				}
			}
		}
	}
}

void FVersionInstallationManage::Wait(float& InProgressInstallationPercentage, int32 InPakNumber, int32 InCustomPakNumber)
{
	if (InPakNumber != 0 || InCustomPakNumber != 0)
	{
		//IV.检测百分比的变化
		UE_LOG(LogVersionInstallation, Display, TEXT("4. Delete temporary package."));
		while (!FMath::IsNearlyEqual(InProgressInstallationPercentage, 1.f, 0.001f) && !IsEngineExitRequested())
		{
			UE_LOG(LogVersionInstallation, Display, TEXT("Start waiting for program progress...[If the program is stuck here for a long time, it may be occupied]"));
			UE_LOG(LogVersionInstallation, Display, TEXT("InProgressInstallationPercentage = %f"),InProgressInstallationPercentage);
		
			FPlatformProcess::Sleep(0.5f);
		}
	}
}

void FVersionInstallationManage::DeleteResourceFile(const TArray<FString>& InFoundFiles)
{
	for (auto& Tmp : InFoundFiles)
	{
		DeleteFile(*Tmp);
	}
}

void FVersionInstallationManage::HandleDescribe(
	const FString& InProjectToContentPaks,
	const FString& InProjectRootPath,
	const TArray<FRemotePatchFile>& InDiscardInfo)
{
	for (const auto& Tmp : InDiscardInfo)
	{
		if (!Tmp.Name.IsEmpty())
		{
			FString NewPath;
			if (Tmp.InstallDir.IsEmpty())
			{
				NewPath = InProjectToContentPaks;
			}
			else
			{
				NewPath = InProjectRootPath + Tmp.InstallDir;
			}

			NewPath /= Tmp.Name;

			DeleteFile(*NewPath);
		}
	}
}

void FVersionInstallationManage::DeleteFile(const TCHAR* NewFilename)
{
	if (IFileManager::Get().Delete(NewFilename))
	{
		UE_LOG(LogVersionInstallation, Display, TEXT("Delete = [%s] Success."), NewFilename);
	}
	else
	{
		UE_LOG(LogVersionInstallation, Error, TEXT("Delete = [%s] Fail."), NewFilename);
	}
}

#undef LOCTEXT_NAMESPACE
