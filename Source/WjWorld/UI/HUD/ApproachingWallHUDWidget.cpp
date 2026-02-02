// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/ApproachingWallHUDWidget.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/GameData/ApproachingWallGameDataComponent.h"
#include "Components/TextBlock.h"

void UApproachingWallHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LevelText)
	{
		LevelText->SetText(FText::FromString(TEXT("Level: 0")));
	}

	if (AlivePlayerText)
	{
		AlivePlayerText->SetText(FText::FromString(TEXT("Alive: 0 / 0")));
	}
}

void UApproachingWallHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	AWjWorldGameStatePlay* GameState = World->GetGameState<AWjWorldGameStatePlay>();
	if (!GameState) return;

	UApproachingWallGameDataComponent* GameData = GameState->GetGameData<UApproachingWallGameDataComponent>();
	if (!GameData) return;

	if (LevelText)
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), GameData->GetCurrentLevel())));
	}

	if (AlivePlayerText)
	{
		AlivePlayerText->SetText(FText::FromString(FString::Printf(TEXT("Alive: %d / %d"), GameData->GetAlivePlayerCount(), GameData->GetTotalPlayerCount())));
	}
}
