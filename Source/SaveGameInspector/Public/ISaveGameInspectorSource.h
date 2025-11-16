// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Features/IModularFeature.h"

class USaveGame;

/** Lightweight description of a save game slot coming from any source. */
struct FSaveGameSlotInfo
{
    /** Logical slot name as expected by UGameplayStatics calls. */
    FString SlotName;

    /** Optional user index, defaults to 0 if unsupported by the source. */
    int32 UserIndex = 0;

    /** Display label shown in the UI. */
    FText DisplayName;

    /** Optional unique identifier within the source (e.g. full path). */
    FString Identifier;

    /** Arbitrary metadata for UI badges/tooltips. */
    TMap<FName, FString> Metadata;
};

/**
 * Extensibility point for SaveGameInspector. Implement and register as a Modular Feature
 * under the feature name GetModularFeatureName().
 *
 * Plugins/projects can provide their own save discovery and persistence logic
 * (cloud saves, profiles, custom formats, etc.).
 */
class ISaveGameInspectorSource : public IModularFeature
{
public:
    static FName GetModularFeatureName()
    {
        static const FName FeatureName(TEXT("SaveGameInspectorSource"));
        return FeatureName;
    }

    virtual ~ISaveGameInspectorSource() = default;

    /** Display name for the source picker (e.g. Local Files, Cloud, Subsystem XYZ). */
    virtual FText GetDisplayName() const = 0;

    /** Optional: Refresh internal cache*/
    virtual void Refresh() {}

    /** Enumerate available slots. Implementations should be fast; cache if needed. Slow loading will freeze the UI */
    virtual void EnumerateSlots(TArray<FSaveGameSlotInfo>& OutSlots) = 0;

    /** Load a USaveGame instance for the slot. Return nullptr on failure. */
    virtual USaveGame* Load(const FSaveGameSlotInfo& Slot) = 0;

    /** Persist the provided save object back to the slot. Return true on success. */
    virtual bool Save(const FSaveGameSlotInfo& Slot, USaveGame* SaveObject) = 0;

    /** Delete the slot. Return true on success. */
    virtual bool Delete(const FSaveGameSlotInfo& Slot) = 0;
};
