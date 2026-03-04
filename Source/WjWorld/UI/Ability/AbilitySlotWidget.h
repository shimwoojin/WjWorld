// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySlotWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UWjWorldGameplayAbilityBase;
class UAbilitySystemComponent;

/**
 * 어빌리티 슬롯 위젯
 * - 어빌리티 아이콘, 키 바인딩, 쿨다운, 충전 정보 표시
 * - UMG Blueprint에서 AbilityClass 설정 후 HUD에 배치
 */
UCLASS()
class WJWORLD_API UAbilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 슬롯에 연결할 어빌리티 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	TSubclassOf<UWjWorldGameplayAbilityBase> AbilityClass;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** 어빌리티 인스턴스 캐시 시도 */
	bool TryCacheAbilityInstance();

	/** ASC에서 AbilityClass에 해당하는 어빌리티 인스턴스 찾기 */
	UWjWorldGameplayAbilityBase* FindAbilityInstance(UAbilitySystemComponent* ASC) const;

	/** DefaultMappingContext에서 이 어빌리티의 키 바인딩 문자열 조회 */
	FText ResolveKeyBindText() const;

	/** 쿨다운/충전 UI 업데이트 */
	void UpdateCooldownDisplay(float DeltaTime);
	void UpdateChargeDisplay();

	/** 아이콘 초기화 */
	void InitializeIcon();

protected:
	// BindWidget 요소 (UMG Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AbilityIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyBindText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> CooldownOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ChargeCountText;

private:
	/** 캐시된 어빌리티 인스턴스 */
	UPROPERTY()
	TWeakObjectPtr<UWjWorldGameplayAbilityBase> CachedAbilityInstance;

	/** 인스턴스 캐시 완료 여부 */
	bool bAbilityCached = false;
};
