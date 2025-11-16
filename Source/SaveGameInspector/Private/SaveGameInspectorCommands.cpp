// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#include "SaveGameInspectorCommands.h"

#define LOCTEXT_NAMESPACE "FSaveGameInspectorModule"

void FSaveGameInspectorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Save Game Inspector", "Bring up Save Game Inspector window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
