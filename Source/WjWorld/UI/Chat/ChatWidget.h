// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "ChatWidget.generated.h"

class UScrollBox;
class UEditableTextBox;

/**
 * 멀티플레이어 채팅 위젯
 * 모든 컨텍스트(Lobby, WaitingRoom, Play)에서 HUDBase를 통해 생성
 */
UCLASS()
class WJWORLD_API UChatWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 채팅 메시지 추가 (UI에 표시) */
	void AddChatMessage(const FString& SenderName, const FString& Message);

	/** 시스템 메시지 추가 (회색 텍스트) */
	void AddSystemMessage(const FString& Message);

	/** 입력 포커스 설정 */
	void FocusChatInput();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ChatInputBox;

private:
	UFUNCTION()
	void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnChatMessageReceived(const FString& SenderName, const FString& Message);

	void SendCurrentMessage();

	static constexpr int32 MaxChatMessages = 50;
};
