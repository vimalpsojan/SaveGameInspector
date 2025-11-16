// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#pragma once

#include "Framework/Commands/Commands.h"
#include "SaveGameInspectorStyle.h"

class FSaveGameInspectorCommands : public TCommands<FSaveGameInspectorCommands>
{
public:

	FSaveGameInspectorCommands()
		: TCommands<FSaveGameInspectorCommands>(TEXT("SaveGameInspector"), NSLOCTEXT("Contexts", "SaveGameInspector", "SaveGameInspector Plugin"), NAME_None, FSaveGameInspectorStyle::GetStyleSetName())
	{
	}
	
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};