// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Net/UnrealNetwork.h"

#include "WjWorldLogCategories.h"

AWjWorldGameStatePlay::AWjWorldGameStatePlay()
{
	bReplicates = true;
	bIsGameStartCountDownReady = false;
}

void AWjWorldGameStatePlay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWjWorldGameStatePlay, CurrentPhase);
	DOREPLIFETIME(AWjWorldGameStatePlay, bIsGameStartCountDownReady);
	DOREPLIFETIME(AWjWorldGameStatePlay, StartCountDownTime);
	DOREPLIFETIME(AWjWorldGameStatePlay, RemainingTime);
}

bool AWjWorldGameStatePlay::HasMatchStarted() const
{
	return CurrentPhase == EGamePhase::Playing;
}

bool AWjWorldGameStatePlay::HasMatchEnded() const
{
	return CurrentPhase == EGamePhase::Finished;
}

void AWjWorldGameStatePlay::GameStartWithCountdown(float CountdownTime)
{
	bIsGameStartCountDownReady = true;
	StartCountDownTime = CountdownTime;
}

UWjWorldGameDataComponent* AWjWorldGameStatePlay::AddGameDataComponent(TSubclassOf<UWjWorldGameDataComponent> InDataComponentClass)
{
	if (InDataComponentClass)
	{
		GameDataComponent = NewObject<UWjWorldGameDataComponent>(this, InDataComponentClass);
		AddInstanceComponent(GameDataComponent);
		GameDataComponent->RegisterComponent();

		return GameDataComponent;
	}

	return nullptr;
}

void AWjWorldGameStatePlay::OnRep_GamePhase()
{
	if(CurrentPhase == EGamePhase::Playing)
	{
		UE_LOG(LogWjWorld, Log, TEXT("GameState: Match Started"));
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameState: Phase changed to %d"), static_cast<int32>(CurrentPhase));
}

void AWjWorldGameStatePlay::OnRep_IsGameStartCountDownReady()
{
	if (bIsGameStartCountDownReady)
	{
		GetWorld()->GetFirstPlayerController()->GetHUD<AWjWorldHUDPlay>()->StartGameStartCountDown(StartCountDownTime);

		UE_LOG(LogWjWorld, Log, TEXT("GameState: Game Start Countdown Ready - %f seconds"), StartCountDownTime);
	}
}
