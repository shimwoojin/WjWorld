// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Chat/ChatWidget.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "WjWorldLogCategories.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력 커밋 바인딩 (Enter 키로 전송)
	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UChatWidget::OnInputCommitted);
	}

	// GameState 채팅 델리게이트 바인딩
	if (UWorld* World = GetWorld())
	{
		if (AWjWorldGameStateBase* GS = Cast<AWjWorldGameStateBase>(World->GetGameState()))
		{
			GS->OnChatMessageReceived.RemoveDynamic(this, &UChatWidget::OnChatMessageReceived);
			GS->OnChatMessageReceived.AddDynamic(this, &UChatWidget::OnChatMessageReceived);
		}
	}
}

void UChatWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (AWjWorldGameStateBase* GS = Cast<AWjWorldGameStateBase>(World->GetGameState()))
		{
			GS->OnChatMessageReceived.RemoveDynamic(this, &UChatWidget::OnChatMessageReceived);
		}
	}

	Super::NativeDestruct();
}

void UChatWidget::AddChatMessage(const FString& SenderName, const FString& Message)
{
	if (!ChatScrollBox)
	{
		return;
	}

	UTextBlock* MsgText = NewObject<UTextBlock>(ChatScrollBox);
	MsgText->SetText(FText::FromString(FString::Printf(TEXT("[%s] %s"), *SenderName, *Message)));
	MsgText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

	FSlateFontInfo FontInfo = MsgText->GetFont();
	FontInfo.Size = 14;
	MsgText->SetFont(FontInfo);

	ChatScrollBox->AddChild(MsgText);
	ChatScrollBox->ScrollToEnd();

	// 메시지 수 제한
	while (ChatScrollBox->GetChildrenCount() > MaxChatMessages)
	{
		ChatScrollBox->RemoveChildAt(0);
	}
}

void UChatWidget::AddSystemMessage(const FString& Message)
{
	if (!ChatScrollBox)
	{
		return;
	}

	UTextBlock* MsgText = NewObject<UTextBlock>(ChatScrollBox);
	MsgText->SetText(FText::FromString(Message));
	MsgText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));

	FSlateFontInfo FontInfo = MsgText->GetFont();
	FontInfo.Size = 14;
	MsgText->SetFont(FontInfo);

	ChatScrollBox->AddChild(MsgText);
	ChatScrollBox->ScrollToEnd();

	while (ChatScrollBox->GetChildrenCount() > MaxChatMessages)
	{
		ChatScrollBox->RemoveChildAt(0);
	}
}

void UChatWidget::FocusChatInput()
{
	if (ChatInputBox)
	{
		ChatInputBox->SetKeyboardFocus();
	}
}

bool UChatWidget::IsChatInputFocused() const
{
	return ChatInputBox && ChatInputBox->HasKeyboardFocus();
}

void UChatWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SendCurrentMessage();
	}
}

void UChatWidget::OnChatMessageReceived(const FString& SenderName, const FString& Message)
{
	AddChatMessage(SenderName, Message);
}

void UChatWidget::SendCurrentMessage()
{
	if (!ChatInputBox)
	{
		return;
	}

	FString Message = ChatInputBox->GetText().ToString().TrimStartAndEnd();
	if (Message.IsEmpty())
	{
		RestoreGameFocus();
		return;
	}

	// PlayerController를 통해 서버로 전송
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AWjWorldPlayerControllerBase* WjPC = Cast<AWjWorldPlayerControllerBase>(PC))
		{
			WjPC->SendChatMessage(Message);
		}
	}

	// 입력 필드 초기화 + 포커스를 게임으로 복원
	ChatInputBox->SetText(FText::GetEmpty());
	RestoreGameFocus();
}

void UChatWidget::RestoreGameFocus()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
}
