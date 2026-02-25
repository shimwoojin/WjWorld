// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Currency/WjWorldCurrencyTypes.h"
#include "CoinGainNotificationWidget.generated.h"

class UTextBlock;

/**
 * Coin/Gem 획득 시 화면에 "+X Coin" 토스트 표시
 * HUDBase에서 생성되어 모든 컨텍스트(Lobby, WaitingRoom, Play)에서 동작
 */
UCLASS()
class WJWORLD_API UCoinGainNotificationWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NotificationText;

private:
	UFUNCTION()
	void OnCurrencyBalanceChanged(ECurrencyType CurrencyType, int32 NewBalance);

	void ShowNotification(const FString& Message);
	void HideNotification();

	int32 CachedCoinBalance = 0;
	int32 CachedGemBalance = 0;

	FTimerHandle HideTimerHandle;
	static constexpr float DisplayDuration = 3.0f;
};
