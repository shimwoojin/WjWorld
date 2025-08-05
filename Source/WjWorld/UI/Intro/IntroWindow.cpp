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

    // MediaTexture�� Image ������ ����
    SetupVideoImage();

    // �̵�� �÷��̾� �̺�Ʈ ���ε�
    BindMediaPlayerEvents();
}

void UIntroWindow::SetupVideoImage()
{
    if (VideoImage && MediaTexture)
    {
        FSlateBrush Brush = VideoImage->GetBrush();
        Brush.SetResourceObject(MediaTexture);
        Brush.ImageSize = FVector2D(1920, 1080); // ������ �ػ󵵿� �°� ����
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
        // �̵�� �ҽ� ���� �� ���
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
        // ��� �Ϸ� �̺�Ʈ ���ε�
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

    // �̺�Ʈ ��ε�ĳ��Ʈ
    OnVideoCompleted.Broadcast();
}