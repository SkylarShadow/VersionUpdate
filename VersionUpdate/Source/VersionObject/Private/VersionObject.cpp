

#include "VersionObject.h"

#define LOCTEXT_NAMESPACE "VersionObject"

void FVersionObjectModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

}

void FVersionObjectModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVersionObjectModule, VersionObject)