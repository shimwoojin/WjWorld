// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Session/PasswordInputWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "WjWorldLogCategories.h"

void UPasswordInputWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SubmitButton)
	{
		SubmitButton->OnClicked.AddDynamic(this, &UPasswordInputWidget::OnSubmitClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UPasswordInputWidget::OnCancelClicked);
	}

	if (PasswordTextBox)
	{
		PasswordTextBox->SetIsPassword(true);
		PasswordTextBox->OnTextCommitted.AddDynamic(this, &UPasswordInputWidget::OnPasswordTextCommitted);
	}

	// NativeConstruct 이전에 SetMessage가 호출되었으면 재적용
	if (bMessageSet && MessageText)
	{
		MessageText->SetText(CachedMessage);
	}

	// 에러 텍스트 초기 숨김
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
	}
}

void UPasswordInputWidget::ShowPopup()
{
	AddToViewport(200);

	// 비밀번호 필드 초기화 및 포커스
	if (PasswordTextBox)
	{
		PasswordTextBox->SetText(FText::GetEmpty());
		PasswordTextBox->SetKeyboardFocus();
	}

	// 에러 텍스트 초기화
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
	}
}

void UPasswordInputWidget::ClosePopup()
{
	RemoveFromParent();
}

void UPasswordInputWidget::SetMessage(const FText& InMessage)
{
	CachedMessage = InMessage;
	bMessageSet = true;

	if (MessageText)
	{
		MessageText->SetText(InMessage);
	}
}

void UPasswordInputWidget::SetErrorMessage(const FText& InMessage)
{
	if (ErrorText)
	{
		ErrorText->SetText(InMessage);
	}
}

void UPasswordInputWidget::OnSubmitClicked()
{
	if (!PasswordTextBox)
	{
		return;
	}

	FString Password = PasswordTextBox->GetText().ToString();

	if (Password.IsEmpty())
	{
		SetErrorMessage(FText::FromString(TEXT("Please enter a password.")));
		return;
	}

	UE_LOG(LogWjWorld, Log, TEXT("PasswordInputWidget: Password submitted"));
	OnPasswordSubmitted.Broadcast(Password);
}

void UPasswordInputWidget::OnCancelClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("PasswordInputWidget: Cancelled"));
	OnPasswordCancelled.Broadcast();
	ClosePopup();
}

void UPasswordInputWidget::OnPasswordTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Enter 키로 제출
	if (CommitMethod == ETextCommit::OnEnter)
	{
		OnSubmitClicked();
	}
}
