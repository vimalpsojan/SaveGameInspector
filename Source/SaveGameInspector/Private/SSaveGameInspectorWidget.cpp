// Copyright (c) 2025 Vimal P Sojan
// Licensed under the MIT License. See LICENSE file in the project root.

#include "SSaveGameInspectorWidget.h"

#include "ISaveGameInspectorSource.h"
#include "Features/IModularFeatures.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/SaveGame.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"

SSaveGameInspectorWidget::FExtendToolbarDelegate& SSaveGameInspectorWidget::GetToolbarExtender()
{
    static FExtendToolbarDelegate Delegate;
    return Delegate;
}

SSaveGameInspectorWidget::FExtendSlotContextMenuDelegate& SSaveGameInspectorWidget::GetSlotContextMenuExtender()
{
    static FExtendSlotContextMenuDelegate Delegate;
    return Delegate;
}

void SSaveGameInspectorWidget::Construct(const FArguments& InArgs)
{
    RefreshSources();

    // Create details panel for any USaveGame
    FPropertyEditorModule& PropModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = true;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.bAllowFavoriteSystem = true;
    DetailsArgs.NotifyHook = nullptr;
    DetailsView = PropModule.CreateDetailView(DetailsArgs);

    // toolbar
    TSharedRef<SWidget> ToolbarWidget = SNullWidget::NullWidget;
    {
        FToolBarBuilder ToolbarBuilder(nullptr, FMultiBoxCustomization::None);
        ToolbarBuilder.BeginSection("SaveGameInspectorActions");
        {
            ToolbarBuilder.AddToolBarButton(
                FUIAction(FExecuteAction::CreateLambda([this]() { OnRefreshClicked(); })),
                NAME_None, FText::FromString("Refresh"), FText::FromString("Refresh sources and slots"));

            ToolbarBuilder.AddToolBarButton(
                FUIAction(FExecuteAction::CreateLambda([this]() { OnReloadClicked(); }), FCanExecuteAction::CreateSP(this, &SSaveGameInspectorWidget::CanModify)),
                NAME_None, FText::FromString("Reload"), FText::FromString("Reload selected slot"));

            ToolbarBuilder.AddToolBarButton(
                FUIAction(FExecuteAction::CreateLambda([this]() { OnSaveClicked(); }), FCanExecuteAction::CreateSP(this, &SSaveGameInspectorWidget::CanModify)),
                NAME_None, FText::FromString("Save"), FText::FromString("Save current object back to slot"));

            ToolbarBuilder.AddToolBarButton(
                FUIAction(FExecuteAction::CreateLambda([this]() { OnDeleteClicked(); }), FCanExecuteAction::CreateLambda([this]() { return SelectedSlot.IsValid() && CurrentSource.IsValid(); })),
                NAME_None, FText::FromString("Delete"), FText::FromString("Delete selected slot"));
        }
        ToolbarBuilder.EndSection();

        // for external extensions
        GetToolbarExtender().Broadcast(ToolbarBuilder);

        ToolbarWidget = ToolbarBuilder.MakeWidget();
    }

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(4)
        [
            ToolbarWidget
        ]

        + SVerticalBox::Slot().FillHeight(1.0f).Padding(4)
        [
            SNew(SSplitter)
            + SSplitter::Slot().Value(0.35f)
            [
                SNew(SBorder)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(4)
                    [
                        SAssignNew(SourceCombo, SComboBox<TSharedPtr<FSourceEntry>>)
                        .OptionsSource(&Sources)
                        .OnGenerateWidget(this, &SSaveGameInspectorWidget::MakeSourceComboItem)
                        .OnSelectionChanged(this, &SSaveGameInspectorWidget::OnSourceSelected)
                        [
                            SNew(STextBlock).Text(this, &SSaveGameInspectorWidget::GetCurrentSourceText)
                        ]
                    ]
                    + SVerticalBox::Slot().FillHeight(1.0f).Padding(4)
                    [
                        SAssignNew(SlotListView, SListView<TSharedPtr<FSaveGameSlotInfo>>)
                        .ListItemsSource(&SlotItems)
                        .SelectionMode(ESelectionMode::Single)
                        .OnGenerateRow(this, &SSaveGameInspectorWidget::OnGenerateSlotRow)
                        .OnSelectionChanged(this, &SSaveGameInspectorWidget::OnSlotSelectionChanged)
                        .OnContextMenuOpening(this, &SSaveGameInspectorWidget::OnGetSlotContextMenu)
                    ]
                ]
            ]
            + SSplitter::Slot().Value(0.65f)
            [
                SNew(SBorder)
                [
                    DetailsView.ToSharedRef()
                ]
            ]
        ]
    ];

    // Preselect first source
    if (!Sources.IsEmpty())
    {
        CurrentSource = Sources[0];
        if (SourceCombo.IsValid())
        {
            SourceCombo->SetSelectedItem(CurrentSource);
        }
    }
    RefreshSlots();
}

void SSaveGameInspectorWidget::RefreshSources()
{
    Sources.Reset();

    TArray<ISaveGameInspectorSource*> FoundSources = IModularFeatures::Get().GetModularFeatureImplementations<ISaveGameInspectorSource>(ISaveGameInspectorSource::GetModularFeatureName());
    for (ISaveGameInspectorSource* Src : FoundSources)
    {
        if (Src)
        {
            TSharedPtr<FSourceEntry> Entry = MakeShared<FSourceEntry>();
            Entry->Ptr = Src;
            Entry->DisplayName = Src->GetDisplayName();
            Sources.Add(Entry);
        }
    }

    if (SourceCombo.IsValid())
    {
        SourceCombo->RefreshOptions();
    }
}

TSharedRef<SWidget> SSaveGameInspectorWidget::MakeSourceComboItem(TSharedPtr<FSourceEntry> InItem) const
{
    return SNew(STextBlock).Text(InItem.IsValid() ? InItem->DisplayName : FText::FromString(TEXT("-")));
}

void SSaveGameInspectorWidget::OnSourceSelected(TSharedPtr<FSourceEntry> InItem, ESelectInfo::Type)
{
    CurrentSource = InItem;
    if (CurrentSource.IsValid() && CurrentSource->Ptr)
    {
        CurrentSource->Ptr->Refresh();
    }
    RefreshSlots();
}

FText SSaveGameInspectorWidget::GetCurrentSourceText() const
{
    return CurrentSource.IsValid() ? CurrentSource->DisplayName : FText::FromString(TEXT("Select Source"));
}

void SSaveGameInspectorWidget::RefreshSlots()
{
    SlotItems.Reset();
    SelectedSlot.Reset();
    ClearLoadedObject();

    if (CurrentSource.IsValid() && CurrentSource->Ptr)
    {
        TArray<FSaveGameSlotInfo> Temp;
        CurrentSource->Ptr->EnumerateSlots(Temp);
        for (const FSaveGameSlotInfo& S : Temp)
        {
            SlotItems.Add(MakeShared<FSaveGameSlotInfo>(S));
        }
    }

    if (SlotListView.IsValid())
    {
        SlotListView->RequestListRefresh();
    }
}

TSharedRef<ITableRow> SSaveGameInspectorWidget::OnGenerateSlotRow(TSharedPtr<FSaveGameSlotInfo> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
    return SNew(STableRow<TSharedPtr<FSaveGameSlotInfo>>, OwnerTable)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(4)
        [
            SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? Item->SlotName : TEXT("")))
        ]
    ];
}

void SSaveGameInspectorWidget::OnSlotSelectionChanged(TSharedPtr<FSaveGameSlotInfo> Item, ESelectInfo::Type)
{
    SelectedSlot = Item;
    LoadSelectedSlot();
}

TSharedPtr<SWidget> SSaveGameInspectorWidget::OnGetSlotContextMenu()
{
    if (!SelectedSlot.IsValid())
    {
        return nullptr;
    }

    FMenuBuilder MenuBuilder(true, nullptr);
    MenuBuilder.AddMenuEntry(
        FText::FromString("Reload"),
        FText::FromString("Reload this save from source"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([this]() { OnReloadClicked(); }))
    );

    MenuBuilder.AddMenuEntry(
        FText::FromString("Save"),
        FText::FromString("Save current object to this slot"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([this]() { OnSaveClicked(); }), FCanExecuteAction::CreateLambda([this]() { return CanModify(); }))
    );

    MenuBuilder.AddMenuEntry(
        FText::FromString("Delete"),
        FText::FromString("Delete this slot"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([this]() { OnDeleteClicked(); }))
    );

    // Allow external extensions
    GetSlotContextMenuExtender().Broadcast(*SelectedSlot.Get(), MenuBuilder);

    return MenuBuilder.MakeWidget();
}

FReply SSaveGameInspectorWidget::OnRefreshClicked()
{
    RefreshSources();
    RefreshSlots();
    return FReply::Handled();
}

FReply SSaveGameInspectorWidget::OnReloadClicked()
{
    LoadSelectedSlot();
    return FReply::Handled();
}

FReply SSaveGameInspectorWidget::OnSaveClicked()
{
    if (CurrentSource.IsValid() && CurrentSource->Ptr && SelectedSlot.IsValid() && LoadedSaveObject.IsValid())
    {
        CurrentSource->Ptr->Save(*SelectedSlot.Get(), LoadedSaveObject.Get());
    }
    return FReply::Handled();
}

FReply SSaveGameInspectorWidget::OnDeleteClicked()
{
    if (CurrentSource.IsValid() && CurrentSource->Ptr && SelectedSlot.IsValid())
    {
        CurrentSource->Ptr->Delete(*SelectedSlot.Get());
        RefreshSlots();
    }
    return FReply::Handled();
}

bool SSaveGameInspectorWidget::CanModify() const
{
    return LoadedSaveObject.IsValid();
}

void SSaveGameInspectorWidget::LoadSelectedSlot()
{
    ClearLoadedObject();

    if (CurrentSource.IsValid() && CurrentSource->Ptr && SelectedSlot.IsValid())
    {
        if (USaveGame* Loaded = CurrentSource->Ptr->Load(*SelectedSlot.Get()))
        {
            LoadedSaveObject.Reset(Loaded);
        }
    }

    if (DetailsView.IsValid())
    {
        DetailsView->SetObject(LoadedSaveObject.Get());
    }
}

void SSaveGameInspectorWidget::ClearLoadedObject()
{
    if (LoadedSaveObject.IsValid())
    {
        LoadedSaveObject.Reset();
    }

    if (DetailsView.IsValid())
    {
        DetailsView->SetObject(nullptr);
    }
}
