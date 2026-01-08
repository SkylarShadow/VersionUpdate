// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Version/CtrlVersion.h"

FString LinkURL;
FString ResourcesToCopiedCustom;
FString ResourcesToCopied;
FString ProjectToContentPaks;
FString ProjectStartupPath;
FString ProjectRootPath;
TArray<FString> RelativePatchs;
uint32 ProjectProcessID = 0;
float ProgressInstallationPercentage = 0.f;
//TArray<FRemotePatchFile> DiscardInfo;
TArray<FString> DiscardPaths;
