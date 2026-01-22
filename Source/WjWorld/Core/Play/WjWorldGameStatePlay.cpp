// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "WjWorldLogCategories.h"

AWjWorldGameStatePlay::AWjWorldGameStatePlay()
{
	bReplicates = true;
	bIsGameStartCountDownReady = false;
	StartCountDownTime = 0.0f;
	CurrentPhase = EGamePhase::None;
}

void AWjWorldGameStatePlay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWjWorldGameStatePlay, CurrentPhase);
	DOREPLIFETIME(AWjWorldGameStatePlay, bIsGameStartCountDownReady);
	DOREPLIFETIME(AWjWorldGameStatePlay, StartCountDownTime);
	DOREPLIFETIME(AWjWorldGameStatePlay, bIsGameEndCountDownReady);
	DOREPLIFETIME(AWjWorldGameStatePlay, EndCountDownTime);
	DOREPLIFETIME(AWjWorldGameStatePlay, WinnerPlayerName);
	DOREPLIFETIME(AWjWorldGameStatePlay, bGameHasWinner);
	DOREPLIFETIME(AWjWorldGameStatePlay, bGameResultReady);
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

	if(HasAuthority())
	{
		OnRep_IsGameStartCountDownReady();
	}
}

void AWjWorldGameStatePlay::GameEndWithCountdown(float CountdownTime)
{
	bIsGameEndCountDownReady = true;
	EndCountDownTime = CountdownTime;

	if (HasAuthority())
	{
		OnRep_IsGameEndCountDownReady();
	}
}

void AWjWorldGameStatePlay::SetGamePhase(EGamePhase NewPhase)
{
	CurrentPhase = NewPhase;
	if (HasAuthority())
	{
		OnRep_GamePhase();
	}
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
	switch (CurrentPhase)
	{
	case EGamePhase::Ready:
		UE_LOG(LogWjWorld, Log, TEXT("GameState: Match Ready"));
		break;
	case EGamePhase::Playing:
		UE_LOG(LogWjWorld, Log, TEXT("GameState: Match Started"));
		break;
	case EGamePhase::Finished:
		// 결과 표시는 OnRep_GameResult에서 처리
		UE_LOG(LogWjWorld, Log, TEXT("GameState: Match Finished"));
		break;
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

void AWjWorldGameStatePlay::OnRep_IsGameEndCountDownReady()
{
	if (bIsGameEndCountDownReady)
	{
		GetWorld()->GetFirstPlayerController()->GetHUD<AWjWorldHUDPlay>()->StartGameStartCountDown(EndCountDownTime);

		UE_LOG(LogWjWorld, Log, TEXT("GameState: Game End Countdown Ready - %f seconds"), EndCountDownTime);
	}
}

void AWjWorldGameStatePlay::SetGameResult(const FString& WinnerName, bool bHasWinner)
{
	WinnerPlayerName = WinnerName;
	bGameHasWinner = bHasWinner;
	bGameResultReady = true;

	if (HasAuthority())
	{
		OnRep_GameResult();
	}
}

void AWjWorldGameStatePlay::OnRep_GameResult()
{
	if (!bGameResultReady) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	AWjWorldHUDPlay* HUD = PC->GetHUD<AWjWorldHUDPlay>();
	if (!HUD) return;

	FString ResultText;
	if (bGameHasWinner)
	{
		// 현재 플레이어가 승자인지 확인
		FString LocalPlayerName;
		if (PC->PlayerState)
		{
			LocalPlayerName = PC->PlayerState->GetPlayerName();
		}

		if (LocalPlayerName == WinnerPlayerName)
		{
			ResultText = TEXT("Victory!");
		}
		else
		{
			ResultText = FString::Printf(TEXT("%s Wins!"), *WinnerPlayerName);
		}
	}
	else
	{
		ResultText = TEXT("Draw!");
	}

	HUD->ShowGameResultText(ResultText, 5.0f);

	UE_LOG(LogWjWorld, Log, TEXT("GameState: Game Result - %s"), *ResultText);
}
