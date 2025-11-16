// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#include "SaveGameInspector.h"
#include "SaveGameInspectorStyle.h"
#include "SaveGameInspectorCommands.h"
#include "ISaveGameInspectorSource.h"
#include "LocalSaveGameInspectorSource.h"
#include "SSaveGameInspectorWidget.h"
#include "Features/IModularFeatures.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName SaveGameInspectorTabName("SaveGameInspector");

#define LOCTEXT_NAMESPACE "FSaveGameInspectorModule"

void FSaveGameInspectorModule::StartupModule()
{
	
	FSaveGameInspectorStyle::Initialize();
	FSaveGameInspectorStyle::ReloadTextures();

	FSaveGameInspectorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSaveGameInspectorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FSaveGameInspectorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSaveGameInspectorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SaveGameInspectorTabName, FOnSpawnTab::CreateRaw(this, &FSaveGameInspectorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FSaveGameInspectorTabTitle", "Save Game Inspector"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Register default local source as a modular feature so projects can add their own sources
	LocalSource = new FLocalSaveGameInspectorSource();
	IModularFeatures::Get().RegisterModularFeature(ISaveGameInspectorSource::GetModularFeatureName(), LocalSource);
}

void FSaveGameInspectorModule::ShutdownModule()
{

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSaveGameInspectorStyle::Shutdown();

	FSaveGameInspectorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SaveGameInspectorTabName);
	
	if (LocalSource)
	{
		IModularFeatures::Get().UnregisterModularFeature(ISaveGameInspectorSource::GetModularFeatureName(), LocalSource);
		delete LocalSource;
		LocalSource = nullptr;
	}
}

TSharedRef<SDockTab> FSaveGameInspectorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SSaveGameInspectorWidget)
		];
}

void FSaveGameInspectorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SaveGameInspectorTabName);
}

void FSaveGameInspectorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSaveGameInspectorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSaveGameInspectorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSaveGameInspectorModule, SaveGameInspector);