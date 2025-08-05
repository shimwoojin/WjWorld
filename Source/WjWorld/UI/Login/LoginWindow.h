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
    // 로그인 진행 상태 업데이트
    UFUNCTION(BlueprintCallable)
    void UpdateLoginProgress(float Progress);

    // 로그인 상태 텍스트 업데이트
    UFUNCTION(BlueprintCallable)
    void UpdateLoginStatusText(const FString& StatusText);

protected:
    virtual void NativeConstruct() override;

    // 로그인 상태 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoginStatusText;

    // 로그인 진행 바 (선택사항)
    UPROPERTY(meta = (BindWidget))
    UProgressBar* LoginProgressBar;

private:
    // 진행 상태 애니메이션용 타이머
    FTimerHandle ProgressTimerHandle;
    float CurrentProgress = 0.0f;

    // 진행 바 업데이트
    void UpdateProgressBar();
};
