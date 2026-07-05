// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VersionUpdate : ModuleRules
{
	public VersionUpdate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bool bUseHotPatcherPakApi = true;
		PublicDefinitions.Add("VERSIONUPDATE_USE_HOTPATCHER_PAK=" + (bUseHotPatcherPakApi ? "1" : "0"));
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "HTTP",
                "UnHTTP",
                "VersionPak",
                "VersionManifest",
                "VersionInstallation"
				// ... add other public dependencies that you statically link with here ...
			}
			);

		if (bUseHotPatcherPakApi)
		{
			PublicDependencyModuleNames.Add("HotPatcherRuntime");
		}
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "Slate",
                "SlateCore",
                "Json",
                "EngineSettings"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
