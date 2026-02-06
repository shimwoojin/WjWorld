// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Lobby/PlacementHUDWidget.h"
#include "Core/Local/Lobby/WjWorldGameModeLobby.h"
#include "WjWorldLogCategories.h"

void UPlacementHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 로비 배치 모드 타이틀 설정 (선택적)
	SetTitleText(FText::FromString(TEXT("로비 배치 모드")));
}

void UPlacementHUDWidget::OnExitClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidget: Exit clicked - returning to lobby"));

	AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ExitPlacementMode();
	}
}
