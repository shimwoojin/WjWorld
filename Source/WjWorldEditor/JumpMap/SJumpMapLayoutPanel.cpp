// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpMap/SJumpMapLayoutPanel.h"
#include "JumpMap/JumpMapLevelEditorSubsystem.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "SJumpMapLayoutPanel"

void SJumpMapLayoutPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		// 타이틀
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 8.f, 8.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Title", "JumpMap Layout Editor"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 0.f)
		[
			SNew(SSeparator)
		]

		// 레이아웃 이름 입력
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 8.f, 8.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LayoutNameLabel", "Layout Name:"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(LayoutNameInput, SEditableTextBox)
				.HintText(LOCTEXT("LayoutNameHint", "Enter layout name..."))
			]
		]

		// 버튼 행
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveButton", "Save Layout"))
				.OnClicked(this, &SJumpMapLayoutPanel::OnSaveClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearButton", "Clear All Actors"))
				.OnClicked(this, &SJumpMapLayoutPanel::OnClearClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 8.f, 8.f, 4.f)
		[
			SNew(SSeparator)
		]

		// 사용 가능한 레이아웃 섹션
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AvailableLayouts", "Available Layouts"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		// 레이아웃 목록
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(8.f, 4.f)
		[
			SNew(SBox)
			.MinDesiredHeight(200.f)
			[
				SAssignNew(LayoutListView, SListView<TSharedPtr<FString>>)
				.ListItemsSource(&LayoutNames)
				.OnGenerateRow(this, &SJumpMapLayoutPanel::OnGenerateRow)
			]
		]

		// 하단 버튼 + 상태
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f, 8.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshButton", "Refresh"))
				.OnClicked(this, &SJumpMapLayoutPanel::OnRefreshClicked)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SJumpMapLayoutPanel::GetStatusText)
			]
		]
	];

	// 초기 레이아웃 목록 로드
	RefreshLayoutList();
}

FReply SJumpMapLayoutPanel::OnSaveClicked()
{
	FString LayoutName = LayoutNameInput->GetText().ToString().TrimStartAndEnd();
	if (LayoutName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpMapLayoutPanel: Layout name is empty"));
		return FReply::Handled();
	}

	UJumpMapLevelEditorSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->SaveCurrentLevelLayout(LayoutName);
		RefreshLayoutList();
	}

	return FReply::Handled();
}

FReply SJumpMapLayoutPanel::OnClearClicked()
{
	UJumpMapLevelEditorSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->ClearAllJumpMapActors();
	}

	return FReply::Handled();
}

FReply SJumpMapLayoutPanel::OnRefreshClicked()
{
	RefreshLayoutList();
	return FReply::Handled();
}

FReply SJumpMapLayoutPanel::OnLoadLayoutClicked(TSharedPtr<FString> LayoutName)
{
	if (!LayoutName.IsValid()) return FReply::Handled();

	UJumpMapLevelEditorSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		Subsystem->LoadLayoutToCurrentLevel(*LayoutName);
	}

	return FReply::Handled();
}

void SJumpMapLayoutPanel::RefreshLayoutList()
{
	LayoutNames.Empty();

	UJumpMapLevelEditorSubsystem* Subsystem = GetSubsystem();
	if (Subsystem)
	{
		TArray<FString> Names = Subsystem->GetAvailableLayoutNames();
		for (const FString& Name : Names)
		{
			LayoutNames.Add(MakeShared<FString>(Name));
		}
	}

	if (LayoutListView.IsValid())
	{
		LayoutListView->RequestListRefresh();
	}
}

FText SJumpMapLayoutPanel::GetStatusText() const
{
	UJumpMapLevelEditorSubsystem* Subsystem = GetSubsystem();
	int32 ActorCount = Subsystem ? Subsystem->GetJumpMapActorCount() : 0;
	return FText::Format(LOCTEXT("StatusFormat", "Status: {0} actors in current level"), FText::AsNumber(ActorCount));
}

TSharedRef<ITableRow> SJumpMapLayoutPanel::OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(4.f, 2.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*Item))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadButton", "Load"))
				.OnClicked(this, &SJumpMapLayoutPanel::OnLoadLayoutClicked, Item)
			]
		];
}

UJumpMapLevelEditorSubsystem* SJumpMapLayoutPanel::GetSubsystem() const
{
	if (!GEditor) return nullptr;
	return GEditor->GetEditorSubsystem<UJumpMapLevelEditorSubsystem>();
}

#undef LOCTEXT_NAMESPACE
