// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementLoadDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "WjWorldLogCategories.h"

void UPlacementLoadDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UPlacementLoadDialogWidget::OnCancelClicked);
	}

	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("레이아웃 불러오기")));
	}

	// 기본적으로 안내 텍스트 숨기기
	if (EmptyHintText)
	{
		EmptyHintText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlacementLoadDialogWidget::ShowPopup()
{
	AddToViewport(200);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementLoadDialogWidget: Popup shown"));
}

void UPlacementLoadDialogWidget::ClosePopup()
{
	RemoveFromParent();
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementLoadDialogWidget: Popup closed"));
}

void UPlacementLoadDialogWidget::SetSlotList(const TArray<FString>& SlotNames)
{
	if (!SlotListScrollBox)
	{
		return;
	}

	SlotListScrollBox->ClearChildren();
	ButtonToSlotNameMap.Empty();
	DeleteButtonToSlotNameMap.Empty();

	if (SlotNames.Num() == 0)
	{
		// 저장된 슬롯이 없음
		if (EmptyHintText)
		{
			EmptyHintText->SetText(FText::FromString(TEXT("저장된 레이아웃이 없습니다.")));
			EmptyHintText->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	if (EmptyHintText)
	{
		EmptyHintText->SetVisibility(ESlateVisibility::Collapsed);
	}

	for (const FString& SlotName : SlotNames)
	{
		// 수평 박스: [슬롯 이름 버튼 (Fill) | X 삭제 버튼 (Auto)]
		UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

		// 슬롯 이름 버튼
		UButton* SlotButton = NewObject<UButton>(this);
		UTextBlock* SlotText = NewObject<UTextBlock>(this);
		SlotText->SetText(FText::FromString(SlotName));
		FSlateFontInfo FontInfo = SlotText->GetFont();
		FontInfo.Size = 14;
		SlotText->SetFont(FontInfo);
		SlotButton->AddChild(SlotText);

		ButtonToSlotNameMap.Add(SlotButton, SlotName);
		SlotButton->OnClicked.AddDynamic(this, &UPlacementLoadDialogWidget::OnSlotButtonClicked);

		UHorizontalBoxSlot* SlotButtonSlot = Row->AddChildToHorizontalBox(SlotButton);
		SlotButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// X 삭제 버튼
		UButton* DeleteButton = NewObject<UButton>(this);
		UTextBlock* DeleteText = NewObject<UTextBlock>(this);
		DeleteText->SetText(FText::FromString(TEXT("X")));
		DeleteText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		FSlateFontInfo DeleteFontInfo = DeleteText->GetFont();
		DeleteFontInfo.Size = 14;
		DeleteText->SetFont(DeleteFontInfo);
		DeleteButton->AddChild(DeleteText);

		DeleteButtonToSlotNameMap.Add(DeleteButton, SlotName);
		DeleteButton->OnClicked.AddDynamic(this, &UPlacementLoadDialogWidget::OnDeleteButtonClicked);

		UHorizontalBoxSlot* DeleteButtonSlot = Row->AddChildToHorizontalBox(DeleteButton);
		DeleteButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

		SlotListScrollBox->AddChild(Row);
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementLoadDialogWidget: Slot list populated with %d items"), SlotNames.Num());
}

void UPlacementLoadDialogWidget::SetContext(EPlacementContext InContext)
{
	CurrentContext = InContext;

	// 타이틀 업데이트
	if (TitleText)
	{
		FString TitleStr;
		switch (CurrentContext)
		{
		case EPlacementContext::Lobby:
			TitleStr = TEXT("로비 레이아웃 불러오기");
			break;
		case EPlacementContext::ApproachingWall:
			TitleStr = TEXT("벽 레이아웃 불러오기");
			break;
		case EPlacementContext::JumpMap:
			TitleStr = TEXT("점프맵 레이아웃 불러오기");
			break;
		default:
			TitleStr = TEXT("레이아웃 불러오기");
			break;
		}
		TitleText->SetText(FText::FromString(TitleStr));
	}
}

void UPlacementLoadDialogWidget::OnCancelClicked()
{
	ClosePopup();
}

void UPlacementLoadDialogWidget::OnSlotButtonClicked()
{
	// 호버 상태인 버튼을 찾아 슬롯 이름 조회
	for (const auto& Pair : ButtonToSlotNameMap)
	{
		if (Pair.Key && Pair.Key->IsHovered())
		{
			FString SlotName = Pair.Value;
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementLoadDialogWidget: Slot selected: %s"), *SlotName);
			OnLoadConfirmed.Broadcast(SlotName);
			ClosePopup();
			return;
		}
	}
}

void UPlacementLoadDialogWidget::OnDeleteButtonClicked()
{
	// 호버 상태인 삭제 버튼을 찾아 슬롯 이름 조회
	for (const auto& Pair : DeleteButtonToSlotNameMap)
	{
		if (Pair.Key && Pair.Key->IsHovered())
		{
			FString SlotName = Pair.Value;
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementLoadDialogWidget: Delete requested for slot: %s"), *SlotName);
			OnSlotDeleteRequested.Broadcast(SlotName);
			return;
		}
	}
}
