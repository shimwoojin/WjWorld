// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Common/ConfirmDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UConfirmDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UConfirmDialogWidget::OnConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UConfirmDialogWidget::OnCancelClicked);
	}

	// NativeConstruct 이전에 SetMessage/SetButtonLabels가 호출되었으면 재적용
	if (bMessageSet && MessageText)
	{
		MessageText->SetText(CachedMessage);
	}

	if (bLabelsSet)
	{
		if (ConfirmButtonText)
		{
			ConfirmButtonText->SetText(CachedConfirmLabel);
		}
		if (CancelButtonText)
		{
			CancelButtonText->SetText(CachedCancelLabel);
		}
	}
}

void UConfirmDialogWidget::ShowPopup()
{
	AddToViewport(200);
}

void UConfirmDialogWidget::ClosePopup()
{
	RemoveFromParent();
}

void UConfirmDialogWidget::SetMessage(const FText& InMessage)
{
	CachedMessage = InMessage;
	bMessageSet = true;

	if (MessageText)
	{
		MessageText->SetText(InMessage);
	}
}

void UConfirmDialogWidget::SetButtonLabels(const FText& ConfirmLabel, const FText& CancelLabel)
{
	CachedConfirmLabel = ConfirmLabel;
	CachedCancelLabel = CancelLabel;
	bLabelsSet = true;

	if (ConfirmButtonText)
	{
		ConfirmButtonText->SetText(ConfirmLabel);
	}
	if (CancelButtonText)
	{
		CancelButtonText->SetText(CancelLabel);
	}
}

void UConfirmDialogWidget::OnConfirmClicked()
{
	OnConfirmed.Broadcast();
	ClosePopup();
}

void UConfirmDialogWidget::OnCancelClicked()
{
	OnCancelled.Broadcast();
	ClosePopup();
}
