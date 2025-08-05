// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "IntroWindow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVideoCompleted);

class UMediaPlayer;
class UMediaTexture;
class UMediaSource;
class UImage;

/**
 * 
 */
UCLASS()
class WJWORLD_API UIntroWindow : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()
	
public:
    // ������ �Ϸ� �̺�Ʈ
    UPROPERTY(BlueprintAssignable)
    FOnVideoCompleted OnVideoCompleted;

    // ������ ��� ����
    UFUNCTION(BlueprintCallable)
    void PlayIntroVideo();

protected:
    virtual void NativeConstruct() override;

    // �̵�� �÷��̾�
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Media")
    UMediaPlayer* MediaPlayer;

    // �̵�� �ؽ�ó
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Media")
    UMediaTexture* MediaTexture;

    // �̵�� �ҽ�
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Media")
    UMediaSource* MediaSource;

    // �������� ǥ���� �̹��� ����
    UPROPERTY(meta = (BindWidget))
    UImage* VideoImage;

private:
    // �̵�� �÷��̾� �̺�Ʈ ���ε�
    void BindMediaPlayerEvents();

    // ������ ��� �Ϸ� �� ȣ��
    UFUNCTION()
    void OnMediaPlaybackFinished();
};
