// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Intro/IntroWindow.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSource.h"
#include "Components/Image.h"
#include "Engine/Engine.h"

void UIntroWindow::PlayIntroVideo()
{
    if (MediaPlayer && MediaSource)
    {
        // 미디어 소스 설정 및 재생
        MediaPlayer->OpenSource(MediaSource);
        MediaPlayer->Play();

        UE_LOG(LogTemp, Warning, TEXT("Started playing intro video"));
    }
}

void UIntroWindow::NativeConstruct()
{
	Super::NativeConstruct();
	BindMediaPlayerEvents();
}

void UIntroWindow::BindMediaPlayerEvents()
{
    if (MediaPlayer)
    {
        // 재생 완료 이벤트 바인딩
        MediaPlayer->OnEndReached.AddDynamic(this, &UIntroWindow::OnMediaPlaybackFinished);
    }
}

void UIntroWindow::OnMediaPlaybackFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Media playback finished"));

    // 이벤트 브로드캐스트
    OnVideoCompleted.Broadcast();
}
