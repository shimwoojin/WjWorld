// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "PasswordInputWidget.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPasswordSubmitted, const FString&, Password);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPasswordCancelled);

/**
 * 비밀번호 입력 팝업 위젯
 * 비공개 방 입장 시 비밀번호를 입력받는 다이얼로그
 */
UCLASS()
class WJWORLD_API UPasswordInputWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	/** 팝업 표시 */
	void ShowPopup();

	/** 팝업 닫기 */
	void ClosePopup();

	/** 메시지 텍스트 설정 */
	void SetMessage(const FText& InMessage);

	/** 에러 메시지 표시 */
	void SetErrorMessage(const FText& InMessage);

	/** 비밀번호 제출 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnPasswordSubmitted OnPasswordSubmitted;

	/** 취소 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnPasswordCancelled OnPasswordCancelled;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> PasswordTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SubmitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	/** 에러 메시지 텍스트 (틀린 비밀번호 등) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ErrorText;

private:
	UFUNCTION()
	void OnSubmitClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnPasswordTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/** NativeConstruct 전 호출 대비 캐시 */
	FText CachedMessage;
	bool bMessageSet = false;
};
