

#include "VersionUpdateEditor.h"
#include "VersionUpdateEditorStyle.h"
#include "VersionUpdateEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "ISettingsModule.h"
#include "Widget/SVersionUpdateSlateMain.h"
#include "HAL/IPlatformFileOpenLogWrapper.h"

static const FName VersionUpdateEditorTabName("VersionUpdateEditor");

#define LOCTEXT_NAMESPACE "FVersionUpdateEditorModule"

void FVersionUpdateEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVersionUpdateEditorStyle::Initialize();
	FVersionUpdateEditorStyle::ReloadTextures();

	FVersionUpdateEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVersionUpdateEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FVersionUpdateEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVersionUpdateEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(VersionUpdateEditorTabName, FOnSpawnTab::CreateRaw(this, &FVersionUpdateEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FVersionUpdateEditorTabTitle", "VersionUpdateEditor"))
		.SetMenuType(ETabSpawnerMenuType::Enabled);

}

void FVersionUpdateEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVersionUpdateEditorStyle::Shutdown();

	FVersionUpdateEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VersionUpdateEditorTabName);
}

TSharedRef<SDockTab> FVersionUpdateEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SVersionUpdateSlateMain)
			]
		];
}

void FVersionUpdateEditorModule::PluginButtonClicked()
{
#if (ENGINE_MAJOR_VERSION == 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 2 && ENGINE_PATCH_VERSION >= 7))
	FGlobalTabmanager::Get()->TryInvokeTab(VersionUpdateEditorTabName);
#else
	FGlobalTabmanager::Get()->InvokeTab(FTabId(VersionUpdateEditorTabName));
#endif
}


void FVersionUpdateEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVersionUpdateEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVersionUpdateEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVersionUpdateEditorModule, VersionUpdateEditor)