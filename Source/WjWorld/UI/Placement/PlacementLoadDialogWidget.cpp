// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementLoadDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
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
		UButton* SlotButton = NewObject<UButton>(this);
		UTextBlock* SlotText = NewObject<UTextBlock>(this);

		SlotText->SetText(FText::FromString(SlotName));

		// 버튼 스타일링
		FSlateFontInfo FontInfo = SlotText->GetFont();
		FontInfo.Size = 14;
		SlotText->SetFont(FontInfo);

		SlotButton->AddChild(SlotText);

		// 버튼 → 슬롯 이름 매핑 등록 + 핸들러 바인딩
		ButtonToSlotNameMap.Add(SlotButton, SlotName);
		SlotButton->OnClicked.AddDynamic(this, &UPlacementLoadDialogWidget::OnSlotButtonClicked);

		SlotListScrollBox->AddChild(SlotButton);
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
