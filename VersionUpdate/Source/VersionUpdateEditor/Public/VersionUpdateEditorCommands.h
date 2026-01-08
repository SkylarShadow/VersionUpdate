

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VersionUpdateEditorStyle.h"

class FVersionUpdateEditorCommands : public TCommands<FVersionUpdateEditorCommands>
{
public:

	FVersionUpdateEditorCommands()
		: TCommands<FVersionUpdateEditorCommands>(TEXT("VersionUpdateEditor"), NSLOCTEXT("Contexts", "VersionUpdateEditor", "VersionUpdateEditor Plugin"), NAME_None, FVersionUpdateEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};