
#include "VersionInstallationProgressStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr< FSlateStyleSet > FVersionInstallationProgressStyle::StyleInstance = NULL;

void FVersionInstallationProgressStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FVersionInstallationProgressStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

void FVersionInstallationProgressStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FVersionInstallationProgressStyle::Get()
{
	return *StyleInstance;
}

FName FVersionInstallationProgressStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("VersionInstallationProgressStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon1920x1080(1920.0f, 1080.0f);
TSharedRef<class FSlateStyleSet> FVersionInstallationProgressStyle::Create()
{
	FString ProgressPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Resources"));
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("VersionInstallationProgressStyle"));
	
	Style->SetContentRoot(ProgressPath);

	Style->Set("VersionInstallationProgressStyle.Background", new IMAGE_BRUSH(TEXT("1080x1920"), Icon1920x1080));

	return Style;
}
#undef IMAGE_BRUSH