

#pragma once

#include "Modules/ModuleManager.h"

class FPakPlatformFile;

class FVersionPakModule : public IModuleInterface
{
public:
	FVersionPakModule();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FPakPlatformFile* GetPakPlatformFile();

private:
	FPakPlatformFile* PakPlatformFile;
};
