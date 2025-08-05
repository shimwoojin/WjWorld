// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "LoginWindow.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class WJWORLD_API ULoginWindow : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
    // �α��� ���� ���� ������Ʈ
    UFUNCTION(BlueprintCallable)
    void UpdateLoginProgress(float Progress);

    // �α��� ���� �ؽ�Ʈ ������Ʈ
    UFUNCTION(BlueprintCallable)
    void UpdateLoginStatusText(const FString& StatusText);

protected:
    virtual void NativeConstruct() override;

    // �α��� ���� �ؽ�Ʈ
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoginStatusText;

    // �α��� ���� �� (���û���)
    UPROPERTY(meta = (BindWidget))
    UProgressBar* LoginProgressBar;

private:
    // ���� ���� �ִϸ��̼ǿ� Ÿ�̸�
    FTimerHandle ProgressTimerHandle;
    float CurrentProgress = 0.0f;

    // ���� �� ������Ʈ
    void UpdateProgressBar();
};
