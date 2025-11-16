// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#include "LocalSaveGameInspectorSource.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/SaveGame.h"

static const TCHAR* SaveGameExtension = TEXT("sav");

FString FLocalSaveGameInspectorSource::GetSaveGamesDir()
{
    // Default Save path: ../Saved/SaveGames/
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"));
}

void FLocalSaveGameInspectorSource::EnumerateSlots(TArray<FSaveGameSlotInfo>& OutSlots)
{
    OutSlots.Reset();

    const FString Dir = GetSaveGamesDir();
    IFileManager& FileManager = IFileManager::Get();

    TArray<FString> Files;
    const FString Pattern = FPaths::Combine(Dir, FString::Printf(TEXT("*.%s"), SaveGameExtension));
    FileManager.FindFiles(Files, *Pattern, true, false);

    for (const FString& File : Files)
    {
        // File is like MySlot.sav → slot name is the base filename ie:MySlot
        FString Base = FPaths::GetBaseFilename(File);

        FSaveGameSlotInfo Slot;
        Slot.SlotName = Base;
        Slot.UserIndex = 0;
        Slot.DisplayName = FText::FromString(Base);
        Slot.Identifier = FPaths::Combine(Dir, File);
        OutSlots.Add(MoveTemp(Slot));
    }
}

USaveGame* FLocalSaveGameInspectorSource::Load(const FSaveGameSlotInfo& Slot)
{
    return UGameplayStatics::LoadGameFromSlot(Slot.SlotName, Slot.UserIndex);
}

bool FLocalSaveGameInspectorSource::Save(const FSaveGameSlotInfo& Slot, USaveGame* SaveObject)
{
    if (!SaveObject)
    {
        return false;
    }
    return UGameplayStatics::SaveGameToSlot(SaveObject, Slot.SlotName, Slot.UserIndex);
}

bool FLocalSaveGameInspectorSource::Delete(const FSaveGameSlotInfo& Slot)
{
    return UGameplayStatics::DeleteGameInSlot(Slot.SlotName, Slot.UserIndex);
}
