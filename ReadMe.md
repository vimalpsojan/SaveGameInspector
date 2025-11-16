### Save Game Inspector — How to Use Guide

#### What is it?
SaveGameInspector is an Unreal Editor tool that lets you browse, load, inspect, and edit any `USaveGame` instance directly in the editor. It’s project‑independent and supports extension points so you can plug in custom save sources (e.g., cloud, subsystems) and add your own toolbar or context‑menu actions.

---

### Requirements
- Unreal Engine: 5.7.0 (tested)
- Module type: Editor module
- Your project must produce save files (typically in `Saved\SaveGames`) or a custom source must provide them

---

### Installation
Copy to Plugins folder
- `Plugins\SaveGameInspector`

No additional setup is required. Build your project as usual.

---

### Opening the Window
- In the Editor: Window → SaveGameInspector
- Or use the Play Toolbar button labelled “SaveGameInspector”

This opens a tab with three main areas:
- Top: Toolbar with `Refresh`, `Reload`, `Save`, `Delete`
- Left: Source picker and Slot list
- Right: Details panel for the loaded `USaveGame`

---

### Core Concepts
- Source: A pluggable provider that enumerates save slots and knows how to load/save/delete them (default source: Local Files under `Saved\SaveGames`).
- Slot: A single saved game entry. The default source treats each `*.sav` file’s base name as the `SlotName` with `UserIndex = 0`.
- Loaded Object: The `USaveGame` instance loaded from the selected slot, shown in the Details panel for live editing.

---

### Quick Start
1. Build and open your project in the Editor.
2. Open SaveGameInspector (Window → SaveGameInspector).
3. In the Source combo, keep `Local Files (Saved/SaveGames)` or pick a custom source if present.
4. Select a slot in the list. The right‑hand Details panel shows the `USaveGame` object.
5. Edit fields in the Details panel as needed.
6. Click `Save` to persist the changes back to the same slot.
7. Use `Reload` to discard unsaved edits and re‑load from the source.
8. Use `Delete` to remove the selected slot.
9. Click `Refresh` to rescan sources and slot lists (helpful after creating new saves during PIE or runtime).

Tip: If the slot list is empty, run your game once to create a save or use your existing save pipeline to generate one.

---

### Editing Save Data
- You can edit any field exposed on your `USaveGame` type (C++ or Blueprint) through the standard Unreal Details panel.
- Complex types are supported (structs, arrays, maps, nested objects referenced by your save object).
- Fields marked `Transient` won’t be saved by your game serialization logic; they may still display but won’t persist when your game writes its own save.
- The plugin’s default local source assumes `UserIndex = 0`. If your project uses multiple user profiles or custom naming, consider adding a custom source (see below).

#### Property editability requirements (important)
- To make a value editable in SaveGameInspector, the property must use an Edit visibility flag (e.g., `EditDefaultsOnly` or `EditAnywhere`).
- Properties declared with a Visible-only flag (e.g., `VisibleDefaultsOnly` or `VisibleAnywhere`) will be read-only in the inspector.
- Recommended for save data in C++:
  - `UPROPERTY(EditDefaultsOnly, SaveGame)` or `UPROPERTY(EditAnywhere, SaveGame)`
  - Avoid `VisibleDefaultsOnly`/`VisibleAnywhere` if you want to change the value in the inspector.
- In Blueprints: make the variable Editable (not just Read Only). If you only set “Read Only,” it will appear but cannot be edited.

Example (C++):
```c++
UCLASS()
class UMySaveGame : public ULocalPlayerSaveGame
{
    GENERATED_BODY()

public:
    // Editable in SaveGameInspector and serialized by the SaveGame system
    UPROPERTY(EditDefaultsOnly, SaveGame, Category="Progress")
    int32 PlayerLevel = 1;

    // This will show but be read-only in the inspector
    UPROPERTY(VisibleDefaultsOnly, SaveGame, Category="Progress")
    int32 ReadOnlyExample = 0;
};
```

---

### Default Source: Local Files (Saved/SaveGames)
- Enumerates `*.sav` files in `Project\Saved\SaveGames`.
- Slot name is the base file name; `UserIndex = 0`.
- Uses `UGameplayStatics::LoadGameFromSlot` / `SaveGameToSlot` / `DeleteGameInSlot` under the hood.

---

### Extending: Add Your Own Source
Implement `ISaveGameInspectorSource` and register it as a Modular Feature in your module’s startup. This lets you list custom slots and define how to load/save/delete them (e.g., profiles, cloud, custom formats):

```c++
// MyPlugin/Public/MyCustomSource.h
#include "ISaveGameInspectorSource.h"

class FMyCustomSource : public ISaveGameInspectorSource
{
public:
    virtual FText GetDisplayName() const override { return FText::FromString("My Source"); }

    virtual void Refresh() override { /* refresh internal cache if needed */ }

    virtual void EnumerateSlots(TArray<FSaveGameSlotInfo>& Out) override
    {
        Out.Reset();
        // Discover slots and push them as FSaveGameSlotInfo entries
        // Example:
        // FSaveGameSlotInfo S; S.SlotName = TEXT("ProfileA"); S.UserIndex = 1; S.DisplayName = FText::FromString("Profile A");
        // S.Identifier = TEXT("any-unique-key"); S.Metadata.Add("Mode", "Campaign");
        // Out.Add(MoveTemp(S));
    }

    virtual USaveGame* Load(const FSaveGameSlotInfo& Slot) override
    {
        // Return a USaveGame instance for this slot (deserialize or load however you store it)
        return nullptr;
    }

    virtual bool Save(const FSaveGameSlotInfo& Slot, USaveGame* SaveObject) override
    {
        // Persist SaveObject back to this slot
        return false;
    }

    virtual bool Delete(const FSaveGameSlotInfo& Slot) override
    {
        // Delete the slot
        return false;
    }
};
```

Register it during your module startup:

```c++
// MyPlugin/Private/MyPluginModule.cpp
#include "Features/IModularFeatures.h"
#include "ISaveGameInspectorSource.h"
#include "MyCustomSource.h"

FMyCustomSource GMySource; // lifetime owned by your module

void FMyPluginModule::StartupModule()
{
    IModularFeatures::Get().RegisterModularFeature(ISaveGameInspectorSource::GetModularFeatureName(), &GMySource);
}

void FMyPluginModule::ShutdownModule()
{
    IModularFeatures::Get().UnregisterModularFeature(ISaveGameInspectorSource::GetModularFeatureName(), &GMySource);
}
```

Once registered, your source appears in the Source combo automatically.

---

### Extending: Add Toolbar and Context‑Menu Actions
You can augment the SaveGameInspector UI from any other plugin or your project by subscribing to its extensibility delegates.

- Extend the toolbar:
```c++
#include "SSaveGameInspectorWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

void RegisterSaveInspectorToolbarExt()
{
    SSaveGameInspectorWidget::GetToolbarExtender().AddLambda([](FToolBarBuilder& TB)
    {
        TB.AddToolBarButton(
            FUIAction(FExecuteAction::CreateLambda([]()
            {
                // Your custom action (e.g., export current save to JSON)
            })),
            NAME_None,
            FText::FromString("Export"),
            FText::FromString("Export current save to JSON"));
    });
}
```

- Extend the per‑slot context menu:
```c++
#include "SSaveGameInspectorWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

void RegisterSaveInspectorSlotMenuExt()
{
    SSaveGameInspectorWidget::GetSlotContextMenuExtender().AddLambda(
        [](const FSaveGameSlotInfo& Slot, FMenuBuilder& Menu)
        {
            Menu.AddMenuEntry(
                FText::FromString("Export JSON"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([Slot]()
                {
                    // Use Slot.Identifier/SlotName/UserIndex to locate and export the save
                }))
            );
        });
}
```

Call your registration functions during your module startup so the actions are available when the window opens.

---

### Troubleshooting
- The window doesn’t appear:
    - Ensure the plugin builds without errors and is enabled (Editor → Plugins → Project → SaveGameInspector).
    - Check the tab under Window → SaveGameInspector; also look for the Play Toolbar button.

- No slots are listed:
    - For the Local Files source, verify saves exist under `Project\Saved\SaveGames`.
    - Click `Refresh` after creating a save in PIE.
    - If your project uses custom naming, multiple `UserIndex` values, or a different storage location, implement a custom source.

- Changes don’t persist after saving:
    - Confirm you clicked `Save` in the inspector after editing.
    - Your game’s own save logic might overwrite fields on next save; ensure fields you want persisted are actually serialized by your game.
    - Fields marked `Transient` are not serialized by `USaveGame`.

- Wrong `USaveGame` type or missing fields:
    - The inspector shows whatever object your source returns. Verify your source loads the expected `USaveGame` subclass.

---

### Safety and Best Practices
- Back up your save directory before bulk editing or deleting.
- Prefer editing in a copy if you’re testing serialization changes.
- If you restructure your `USaveGame` class (add/remove fields), old saves might fail to load or lose data depending on your serialization strategy.

---

### File Reference
- Plugin descriptor: `SaveGameInspector\SaveGameInspector.uplugin`
- Module entry: `SaveGameInspector\Source\SaveGameInspector\Private\SaveGameInspector.cpp`
- Main UI: `...\Private\SSaveGameInspectorWidget.h/.cpp`
- Default local source: `...\Private\LocalSaveGameInspectorSource.h/.cpp`
- Extensibility interface: `...\Public\ISaveGameInspectorSource.h`

---

### Compatibility
- Verified with UE 5.7.0
- Should work with any `USaveGame` subclass (C++ or Blueprint), including nested structs/containers supported by the Details panel.

---

### FAQ
- Can I edit Blueprint `USaveGame` classes? Yes, they appear in the Details panel like any other object.
- Does it support multiple user indices? The default source assumes `UserIndex = 0`. Add a custom source for multi‑profile or non‑standard setups.
- Can I add export/import? Yes, via toolbar and context‑menu extenders. Implement the logic in your project or another plugin.