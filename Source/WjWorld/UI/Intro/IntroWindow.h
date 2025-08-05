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
    // 동영상 완료 이벤트
    UPROPERTY(BlueprintAssignable)
    FOnVideoCompleted OnVideoCompleted;

    // 동영상 재생 시작
    UFUNCTION(BlueprintCallable)
    void PlayIntroVideo();

protected:
    virtual void NativeConstruct() override;

    // 미디어 플레이어
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Media")
    UMediaPlayer* MediaPlayer;

    // 미디어 텍스처
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Media")
    UMediaTexture* MediaTexture;

    // 미디어 소스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Media")
    UMediaSource* MediaSource;

    // 동영상을 표시할 이미지 위젯
    UPROPERTY(meta = (BindWidget))
    UImage* VideoImage;

private:
    // 미디어 플레이어 이벤트 바인딩
    void BindMediaPlayerEvents();

    // 동영상 재생 완료 시 호출
    UFUNCTION()
    void OnMediaPlaybackFinished();
};
