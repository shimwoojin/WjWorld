// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldHUDBase.h"
#include "WjWorldHUDPlay.generated.h"

class UGameplayGlobalHUDWidget;

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldHUDPlay : public AWjWorldHUDBase
{
	GENERATED_BODY()
	
public:
	void StartGameStartCountDown(float CountDown);

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameplayGlobalHUDWidget> GlobalHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UGameplayGlobalHUDWidget> GlobalHUDWidget;
};
