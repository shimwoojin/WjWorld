// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "InteractionWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class WJWORLD_API UInteractionWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractionText(const FString& Text);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* InteractionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* KeyPrompt;
};
