// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldGameModeBase.h"
#include "WjWorldGameModeLogin.generated.h"

class ULoginWindow;

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldGameModeLogin : public AWjWorldGameModeBase
{
	GENERATED_BODY()
	
public:
    AWjWorldGameModeLogin();

protected:
    virtual void BeginPlay() override;

    // Login UI 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<ULoginWindow> LoginWidgetClass;

    // 생성된 위젯 인스턴스
    UPROPERTY()
    ULoginWindow* LoginWidgetInstance;

    // 로그인 대기 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Login")
    float LoginWaitTime = 5.0f;

private:
    // UI 생성 및 표시
    void CreateAndShowLoginUI();

    // 로그인 완료 후 호출될 함수
    void OnLoginCompleted();

    // 타이머 핸들
    FTimerHandle LoginTimerHandle;
};
