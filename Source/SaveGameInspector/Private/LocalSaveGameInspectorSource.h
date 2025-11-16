// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#pragma once

#include "CoreMinimal.h"
#include "ISaveGameInspectorSource.h"

/** Default implementation that scans the project's Saved/SaveGames directory. */
class FLocalSaveGameInspectorSource : public ISaveGameInspectorSource
{
public:
    virtual FText GetDisplayName() const override { return NSLOCTEXT("SaveGameInspector", "LocalSourceName", "Local Files (Saved/SaveGames)"); }

    virtual void EnumerateSlots(TArray<FSaveGameSlotInfo>& OutSlots) override;
    virtual USaveGame* Load(const FSaveGameSlotInfo& Slot) override;
    virtual bool Save(const FSaveGameSlotInfo& Slot, USaveGame* SaveObject) override;
    virtual bool Delete(const FSaveGameSlotInfo& Slot) override;

private:
    static FString GetSaveGamesDir();
};
