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
        // �̵�� �ҽ� ���� �� ���
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
        // ��� �Ϸ� �̺�Ʈ ���ε�
        MediaPlayer->OnEndReached.AddDynamic(this, &UIntroWindow::OnMediaPlaybackFinished);
    }
}

void UIntroWindow::OnMediaPlaybackFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Media playback finished"));

    // �̺�Ʈ ��ε�ĳ��Ʈ
    OnVideoCompleted.Broadcast();
}
