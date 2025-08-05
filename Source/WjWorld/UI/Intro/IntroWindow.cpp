// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Intro/IntroWindow.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSource.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "Styling/SlateBrush.h"

void UIntroWindow::NativeConstruct()
{
    Super::NativeConstruct();

    // MediaTexture를 Image 위젯에 연결
    SetupVideoImage();

    // 미디어 플레이어 이벤트 바인딩
    BindMediaPlayerEvents();
}

void UIntroWindow::SetupVideoImage()
{
    if (VideoImage && MediaTexture)
    {
        FSlateBrush Brush = VideoImage->GetBrush();
        Brush.SetResourceObject(MediaTexture);
        Brush.ImageSize = FVector2D(1920, 1080); // 동영상 해상도에 맞게 조정
        Brush.DrawAs = ESlateBrushDrawType::Image;

        VideoImage->SetBrush(Brush);

        UE_LOG(LogTemp, Warning, TEXT("MediaTexture connected to VideoImage"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VideoImage or MediaTexture is null!"));
    }
}

void UIntroWindow::PlayIntroVideo()
{
    if (MediaPlayer && MediaSource)
    {
        // 미디어 소스 설정 및 재생
        MediaPlayer->OpenSource(MediaSource);
        MediaPlayer->Play();

        UE_LOG(LogTemp, Warning, TEXT("Started playing intro video"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MediaPlayer or MediaSource is null!"));
    }
}

void UIntroWindow::BindMediaPlayerEvents()
{
    if (MediaPlayer)
    {
        // 재생 완료 이벤트 바인딩
        MediaPlayer->OnEndReached.AddDynamic(this, &UIntroWindow::OnMediaPlaybackFinished);

        UE_LOG(LogTemp, Warning, TEXT("MediaPlayer events bound"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MediaPlayer is null - cannot bind events!"));
    }
}

void UIntroWindow::OnMediaPlaybackFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Media playback finished"));

    // 이벤트 브로드캐스트
    OnVideoCompleted.Broadcast();
}