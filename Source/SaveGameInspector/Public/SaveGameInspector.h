// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class SDockTab;
class FUICommandList;
class FSpawnTabArgs;

class FSaveGameInspectorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/** IModuleInterface implementation */
	
	void PluginButtonClicked();
	
private:

    void RegisterMenus();

    TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

private:
    TSharedPtr< FUICommandList> PluginCommands;
	
    class FLocalSaveGameInspectorSource* LocalSource = nullptr;
};
