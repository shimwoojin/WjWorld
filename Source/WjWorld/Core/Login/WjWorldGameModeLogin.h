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

    // Login UI ���� Ŭ����
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<ULoginWindow> LoginWidgetClass;

    // ������ ���� �ν��Ͻ�
    UPROPERTY()
    ULoginWindow* LoginWidgetInstance;

    // �α��� ��� �ð� (��)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Login")
    float LoginWaitTime = 5.0f;

private:
    // UI ���� �� ǥ��
    void CreateAndShowLoginUI();

    // �α��� �Ϸ� �� ȣ��� �Լ�
    void OnLoginCompleted();

    // Ÿ�̸� �ڵ�
    FTimerHandle LoginTimerHandle;
};
