// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeIntro.generated.h"

class UIntroWindow;

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeIntro : public AWjWorldGameModeBase
{
	GENERATED_BODY()
	
public:
	AWjWorldGameModeIntro();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UIntroWindow> IntroWidgetClass;

	UPROPERTY()
	TObjectPtr<UIntroWindow> IntroWidget;

	UFUNCTION()
	void OnVideoFinished();

private:
	// UI 생성 및 표시
	void CreateAndShowIntroUI();
};
