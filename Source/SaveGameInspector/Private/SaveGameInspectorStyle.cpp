// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#include "SaveGameInspectorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSaveGameInspectorStyle::StyleInstance = nullptr;

void FSaveGameInspectorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSaveGameInspectorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSaveGameInspectorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SaveGameInspectorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FSaveGameInspectorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("SaveGameInspectorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SaveGameInspector")->GetBaseDir() / TEXT("Resources"));

	Style->Set("SaveGameInspector.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FSaveGameInspectorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSaveGameInspectorStyle::Get()
{
	return *StyleInstance;
}
