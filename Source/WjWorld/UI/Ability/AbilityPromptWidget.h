// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityPromptWidget.generated.h"

class UTextBlock;

/**
 * Confirm/Cancel 어빌리티 활성화 시 캐릭터 위에 표시되는 프롬프트 위젯
 * - 확인/취소 키 안내 + 어빌리티별 커스텀 설명 표시
 * - CharacterPlay의 WidgetComponent에 설정하여 사용
 */
UCLASS()
class WJWORLD_API UAbilityPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 프롬프트 내용 설정 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Prompt")
	void SetPromptInfo(const FText& ConfirmKeyName, const FText& CancelKeyName, const FText& Description);

protected:
	// BindWidget 요소 (UMG Blueprint에서 바인딩)

	/** 확인 키 안내 텍스트 (예: "좌클릭: 확인") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConfirmText;

	/** 취소 키 안내 텍스트 (예: "우클릭: 취소") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CancelText;

	/** 어빌리티별 커스텀 설명 (예: "벽돌을 배치할 위치를 선택하세요") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;
};
