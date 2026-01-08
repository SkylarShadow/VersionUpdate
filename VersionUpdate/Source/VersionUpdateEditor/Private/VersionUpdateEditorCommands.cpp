

#include "VersionUpdateEditorCommands.h"

#define LOCTEXT_NAMESPACE "FVersionUpdateEditorModule"

void FVersionUpdateEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "VersionUpdateEditor", "Bring up VersionUpdateEditor window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
