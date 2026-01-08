#include "VersionInstallation.h"
#include "Core/Log/VersionInstallationProgressLog.h"
#include "Core/Widget/SMainScreen.h"
#include "VersionInstallationProgressType.h"

float PakNumber = 0.f;
float CustomPakNumber = 0.f;
float LastFraction = 0.f;
float PercentageInterval = 0.f;
namespace VersionInstallation
{
	struct FInstallationProgress :public FCopyProgress
	{
		virtual bool Poll(float Fraction)
		{
			PercentageInterval = Fraction - LastFraction;

			ProgressInstallationPercentage += (PercentageInterval / (PakNumber + CustomPakNumber));

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

	struct FCustomCopyPath
	{
		FString To;
		FString From;
	};

	void DeleteFile(const TCHAR* NewFilename)
	{
		if (IFileManager::Get().Delete(NewFilename))
		{
			UE_LOG(LogVersionInstallationProgress, Display, TEXT("Delete = [%s] Success."), NewFilename);
		}
		else
		{
			UE_LOG(LogVersionInstallationProgress, Error, TEXT("Delete = [%s] Fail."), NewFilename);
		}
	}

	void Run()
	{
		//I.检测游戏是否还在运行
		UE_LOG(LogVersionInstallationProgress, Display, TEXT("1. Check if the game is still running."));
		while (FPlatformProcess::IsApplicationRunning(ProjectProcessID))
		{
			FPlatformProcess::Sleep(1.f);
		}

		//II.资源遍历
		UE_LOG(LogVersionInstallationProgress, Display, TEXT("2. Resource traversal."));
		TArray<FString> FoundFiles;
		if (!ResourcesToCopied.IsEmpty())
		{
			IFileManager::Get().FindFilesRecursive(FoundFiles, *ResourcesToCopied, TEXT("*"), true, false);
		}
		
		//收集自定义拷贝路径
		TArray<FCustomCopyPath> CustomCopyPaths;
		TArray<FString> CustomFoundFiles;
		if (RelativePatchs.Num() > 0)
		{
			IFileManager::Get().FindFilesRecursive(CustomFoundFiles, *ResourcesToCopiedCustom, TEXT("*"), true, false);

			for (auto &Tmp : RelativePatchs)
			{
				FString RelativePatchKey = FPaths::GetCleanFilename(Tmp);
				for (auto &CustomTmp : CustomFoundFiles)
				{
					FString CustomFoundFilesKey = FPaths::GetCleanFilename(CustomTmp);
					if (RelativePatchKey == CustomFoundFilesKey)
					{
						FCustomCopyPath &CopyPath = CustomCopyPaths.AddDefaulted_GetRef();
						CopyPath.From = CustomTmp;
						CopyPath.To = Tmp;
						break;
					}
				}
			}
		}

		PakNumber = FoundFiles.Num();
		CustomPakNumber = CustomCopyPaths.Num();

		TArray<FInstallationProgress*> InstallationProgressArray;

		//III.资源拷贝 //拷贝到pak
		UE_LOG(LogVersionInstallationProgress, Display, TEXT("3. Resource copy."));
		for (auto &Tmp : FoundFiles)
		{
			UE_LOG(LogVersionInstallationProgress, Display, TEXT("Resource = [%s]"),*Tmp);
			FString TargetPath = ProjectToContentPaks / FPaths::GetCleanFilename(Tmp);
			FInstallationProgress *InstallationProgress = new FInstallationProgress();
			IFileManager::Get().Copy(*TargetPath,*Tmp,true,false,false, InstallationProgress);
			
			//作为后面回收
			InstallationProgressArray.Add(InstallationProgress);
		}

		//自定义拷贝
		UE_LOG(LogVersionInstallationProgress, Display, TEXT("3.Custom Resource copy."));
		for (auto &Tmp : CustomCopyPaths)
		{
			UE_LOG(LogVersionInstallationProgress, Display, TEXT("From [%s] => [%s]"),*Tmp.From,*Tmp.To);
			FInstallationProgress* InstallationProgress = new FInstallationProgress();
			IFileManager::Get().Copy(*Tmp.To, *Tmp.From, true, false, false, InstallationProgress);
		
			InstallationProgressArray.Add(InstallationProgress);
		}

		if (PakNumber != 0 || CustomPakNumber != 0)
		{
			//IV.检测百分比的变化
			UE_LOG(LogVersionInstallationProgress, Display, TEXT("4. Delete temporary package."));
			while (!FMath::IsNearlyEqual(ProgressInstallationPercentage, 1.f, 0.001f) && !IsEngineExitRequested())
			{
				FPlatformProcess::Sleep(1.f);
			}
		}

		if (IsEngineExitRequested())
		{
			UE_LOG(LogVersionInstallationProgress, Display, TEXT("5. Program forced exit."));
			return;
		}

		//V.删除临时包
		UE_LOG(LogVersionInstallationProgress, Display, TEXT("5. Delete temporary package."));
		for (auto& Tmp : FoundFiles)
		{
			DeleteFile(*Tmp);
		}

		for (auto &Tmp : CustomFoundFiles)
		{
			DeleteFile(*Tmp);
		}

		//V.删除丢弃的包
		for (auto &TmpPath : DiscardPaths)
		{
			DeleteFile(*TmpPath);

			//Tmp.Link
		}

		//V.启动程序
		UE_LOG(LogVersionInstallationProgress, Display, TEXT("5. Start project."));
		FProcHandle ProjectHandle = FPlatformProcess::CreateProc(*ProjectStartupPath, nullptr, false, false, false, nullptr, 0, nullptr, nullptr);
		if (ProjectHandle.IsValid())
		{
			UE_LOG(LogVersionInstallationProgress, Display, TEXT("Project started successfully."));
		}
		else
		{
			UE_LOG(LogVersionInstallationProgress, Error, TEXT("Failed to start project."));
		}

		FPlatformMisc::RequestExit(true);
		//GIsRequestingExit = true;
	}

	template<class T>
	T GetParseValue(const FString& InKey)
	{
		T Value;
		if (!FParse::Value(FCommandLine::Get(), *InKey, Value))
		{
			UE_LOG(LogVersionInstallationProgress, Error, TEXT("%s was not found value"), *InKey);
		}

		return Value;
	}

	void InitCommandInstallationProgress()
	{
		//提取用户路径
		ResourcesToCopiedCustom = GetParseValue<FString>(TEXT("-ResourcesToCopiedCustom="));
		ResourcesToCopied = GetParseValue<FString>(TEXT("-ResourcesToCopied="));
		ProjectToContentPaks = GetParseValue<FString>(TEXT("-ProjectToContentPaks="));
		ProjectStartupPath = GetParseValue<FString>(TEXT("-ProjectStartupPath="));
		ProjectRootPath = GetParseValue<FString>(TEXT("-ProjectRootPath="));
		LinkURL = GetParseValue<FString>(TEXT("-LinkURL="));
		ProjectProcessID = GetParseValue<uint32>(TEXT("-ProjectProcessID="));

		FString Discards = GetParseValue<FString>(TEXT("-Discards="));

		FString RelativePatchString = GetParseValue<FString>(TEXT("-RelativePatchs="));
		//提取路径
		if (!RelativePatchString.IsEmpty())
		{
			RelativePatchString.ParseIntoArray(RelativePatchs,TEXT("+"));
		}

		//单位化
		FPaths::NormalizeFilename(ResourcesToCopied);
		FPaths::NormalizeFilename(ProjectToContentPaks);
		FPaths::NormalizeFilename(ProjectStartupPath);

		//提取被丢弃的pak
		//TArray<FString> Jsons;
		Discards.ParseIntoArray(DiscardPaths, TEXT("|"));
		//for (auto &Tmp : Jsons)
		//{
		//	auto &RDD = DiscardInfo.AddDefaulted_GetRef();
		//	//VersionControl::Read(Tmp,RDD);
		//}
	}

	TSharedRef<SDockTab> SpawnDockTab(const FSpawnTabArgs& InArgs)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::PanelTab)
			[
				SNew(SMainScreen)
			];
	}

	void SpawnInstallationProgressUI()
	{
		FGlobalTabmanager::Get()->RegisterTabSpawner(TEXT("InstallationProgressUI"), FOnSpawnTab::CreateStatic(&SpawnDockTab));

		TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("InstallationProgressUI_Layout")
		->AddArea
		(
			FTabManager::NewArea(1920, 1080)
			->Split
			(
				FTabManager::NewStack()
				->AddTab("InstallationProgressUI", ETabState::OpenedTab)
			)
		);

		FGlobalTabmanager::Get()->RestoreFrom(Layout, TSharedPtr<SWindow>());
	}
}