// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "ISaveGameInspectorSource.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GameFramework/SaveGame.h"

class IDetailsView;
class USaveGame;

/**
 * Main UI for SaveGameInspector. Lists save slots from a selectable source,
 * loads any USaveGame, and exposes it to a Details panel for inspection/editing.
 */
class SSaveGameInspectorWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SSaveGameInspectorWidget) {}
    SLATE_END_ARGS();

    void Construct(const FArguments& InArgs);

public:
    // Allow other plugins/projects to extend the toolbar with custom actions.
    // Example usage:
    //   SSaveGameInspectorWidget::GetToolbarExtender().AddLambda([](FToolBarBuilder& TB){ TB.AddToolBarButton(...); });
    DECLARE_MULTICAST_DELEGATE_OneParam(FExtendToolbarDelegate, FToolBarBuilder& /*Toolbar Builder*/);
    static FExtendToolbarDelegate& GetToolbarExtender();

    // Allow extending the per-slot context menu (e.g., custom export/import actions).
    DECLARE_MULTICAST_DELEGATE_TwoParams(FExtendSlotContextMenuDelegate, const FSaveGameSlotInfo& /*Slot*/, FMenuBuilder& /*Menu Builder*/);
    static FExtendSlotContextMenuDelegate& GetSlotContextMenuExtender();

private:
    
    struct FSourceEntry
    {
        ISaveGameInspectorSource* Ptr = nullptr;
        FText DisplayName;
    };

    void RefreshSources();
    TSharedRef<SWidget> MakeSourceComboItem(TSharedPtr<FSourceEntry> InItem) const;
    void OnSourceSelected(TSharedPtr<FSourceEntry> InItem, ESelectInfo::Type);
    FText GetCurrentSourceText() const;

    
    void RefreshSlots();
    TSharedRef<ITableRow> OnGenerateSlotRow(TSharedPtr<FSaveGameSlotInfo> Item, const TSharedRef<STableViewBase>& OwnerTable) const;
    void OnSlotSelectionChanged(TSharedPtr<FSaveGameSlotInfo> Item, ESelectInfo::Type);
    TSharedPtr<SWidget> OnGetSlotContextMenu();

    
    FReply OnRefreshClicked();
    FReply OnReloadClicked();
    FReply OnSaveClicked();
    FReply OnDeleteClicked();
    bool  CanModify() const;

    void LoadSelectedSlot();
    void ClearLoadedObject();

private:
    
    TArray<TSharedPtr<FSourceEntry>> Sources;
    TSharedPtr<FSourceEntry> CurrentSource;

    TArray<TSharedPtr<FSaveGameSlotInfo>> SlotItems;
    TSharedPtr<FSaveGameSlotInfo> SelectedSlot;

    // Keep strong ref to prevent GC while editing
    TStrongObjectPtr<USaveGame> LoadedSaveObject;
    
    TSharedPtr<class SComboBox<TSharedPtr<FSourceEntry>>> SourceCombo;
    TSharedPtr<class SListView<TSharedPtr<FSaveGameSlotInfo>>> SlotListView;
    TSharedPtr<IDetailsView> DetailsView;
};
