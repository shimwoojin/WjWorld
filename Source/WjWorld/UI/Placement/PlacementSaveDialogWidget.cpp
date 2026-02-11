// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementSaveDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "WjWorldLogCategories.h"

void UPlacementSaveDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UPlacementSaveDialogWidget::OnConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UPlacementSaveDialogWidget::OnCancelClicked);
	}

	if (SlotNameTextBox)
	{
		SlotNameTextBox->OnTextChanged.AddDynamic(this, &UPlacementSaveDialogWidget::OnSlotNameChanged);
	}

	// CreateWidget → SetContext → SetValidationResult → ShowPopup → NativeConstruct 순서 대응
	// NativeConstruct 이전에 호출된 설정을 위젯 바인딩 완료 후 재적용
	if (bContextSet)
	{
		SetContext(CurrentContext);

		// SetValidationResult도 이미 호출되었으면 재적용
		if (bValidationResultSet)
		{
			SetValidationResult(bIsCurrentlyValid, CachedValidationMessage);
		}
		else
		{
			// validation 미설정 시 기본 활성화
			if (ConfirmButton)
			{
				ConfirmButton->SetIsEnabled(true);
			}
		}
	}
	else
	{
		if (TitleText)
		{
			TitleText->SetText(FText::FromString(TEXT("레이아웃 저장")));
		}

		if (ValidationText)
		{
			ValidationText->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (ValidationStatusText)
		{
			ValidationStatusText->SetVisibility(ESlateVisibility::Collapsed);
		}

		bIsCurrentlyValid = true;
		if (ConfirmButton)
		{
			ConfirmButton->SetIsEnabled(true);
		}
	}
}

void UPlacementSaveDialogWidget::ShowPopup()
{
	AddToViewport(200);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementSaveDialogWidget: Popup shown"));
}

void UPlacementSaveDialogWidget::ClosePopup()
{
	RemoveFromParent();
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementSaveDialogWidget: Popup closed"));
}

void UPlacementSaveDialogWidget::SetDefaultSlotName(const FString& SlotName)
{
	if (SlotNameTextBox)
	{
		SlotNameTextBox->SetText(FText::FromString(SlotName));
	}
}

void UPlacementSaveDialogWidget::SetContext(EPlacementContext InContext)
{
	CurrentContext = InContext;
	bContextSet = true;

	// AW 컨텍스트의 경우 유효성 검사 UI 표시
	bool bShowValidation = (CurrentContext == EPlacementContext::ApproachingWall);

	if (ValidationText)
	{
		ValidationText->SetVisibility(bShowValidation ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (ValidationStatusText)
	{
		ValidationStatusText->SetVisibility(bShowValidation ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 타이틀 업데이트
	if (TitleText)
	{
		FString TitleStr;
		switch (CurrentContext)
		{
		case EPlacementContext::Lobby:
			TitleStr = TEXT("로비 레이아웃 저장");
			break;
		case EPlacementContext::ApproachingWall:
			TitleStr = TEXT("벽 레이아웃 저장");
			break;
		case EPlacementContext::JumpMap:
			TitleStr = TEXT("점프맵 레이아웃 저장");
			break;
		default:
			TitleStr = TEXT("레이아웃 저장");
			break;
		}
		TitleText->SetText(FText::FromString(TitleStr));
	}
}

void UPlacementSaveDialogWidget::SetValidationResult(bool bIsValid, const FText& Message)
{
	bIsCurrentlyValid = bIsValid;
	bValidationResultSet = true;
	CachedValidationMessage = Message;

	if (ValidationText)
	{
		ValidationText->SetText(Message);

		// 유효하면 녹색, 아니면 빨강
		FSlateColor TextColor = bIsValid
			? FSlateColor(FLinearColor::Green)
			: FSlateColor(FLinearColor::Red);
		ValidationText->SetColorAndOpacity(TextColor);
	}

	if (ValidationStatusText)
	{
		ValidationStatusText->SetText(bIsValid
			? FText::FromString(TEXT("[OK]"))
			: FText::FromString(TEXT("[ERROR]")));

		FSlateColor StatusColor = bIsValid
			? FSlateColor(FLinearColor::Green)
			: FSlateColor(FLinearColor::Red);
		ValidationStatusText->SetColorAndOpacity(StatusColor);
	}

	// AW 컨텍스트에서 유효하지 않으면 저장 버튼 비활성화
	if (ConfirmButton && CurrentContext == EPlacementContext::ApproachingWall)
	{
		ConfirmButton->SetIsEnabled(bIsValid);
	}
}

void UPlacementSaveDialogWidget::OnConfirmClicked()
{
	if (!SlotNameTextBox)
	{
		return;
	}

	FString SlotName = SlotNameTextBox->GetText().ToString();

	// 빈 이름 검사
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementSaveDialogWidget: Slot name is empty"));
		return;
	}

	// AW 컨텍스트에서 유효하지 않으면 저장 거부
	if (CurrentContext == EPlacementContext::ApproachingWall && !bIsCurrentlyValid)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementSaveDialogWidget: Cannot save - validation failed"));
		return;
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementSaveDialogWidget: Confirm clicked with slot name: %s"), *SlotName);
	OnSaveConfirmed.Broadcast(SlotName);
	ClosePopup();
}

void UPlacementSaveDialogWidget::OnCancelClicked()
{
	ClosePopup();
}

void UPlacementSaveDialogWidget::OnSlotNameChanged(const FText& Text)
{
	// 슬롯 이름 변경 시 추가 로직 (필요 시)
	UE_LOG(LogWjWorldPlacement, Verbose, TEXT("PlacementSaveDialogWidget: Slot name changed to: %s"), *Text.ToString());
}
