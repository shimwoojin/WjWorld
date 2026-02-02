// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"
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
	DOREPLIFETIME(AWjWorldGameStatePlay, GameRuleClass);
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
		GameDataComponent->SetIsReplicated(true);

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

void AWjWorldGameStatePlay::SetGameRuleClass(TSubclassOf<UWjWorldGameRuleBase> InGameRuleClass)
{
	GameRuleClass = InGameRuleClass;

	if (HasAuthority())
	{
		OnRep_GameRuleClass();
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

	FString LocalPlayerName;
	if (PC->PlayerState)
	{
		LocalPlayerName = PC->PlayerState->GetPlayerName();
	}

	FString ResultText;
	if (bGameHasWinner)
	{
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

	// 스탯 기록 (각 클라이언트가 자신의 스탯만 기록)
	UWjWorldStatsSubsystem* Stats = GetGameInstance()->GetSubsystem<UWjWorldStatsSubsystem>();
	if (Stats && PC->PlayerState)
	{
		Stats->IncrementLocalStat(WjWorldStats::ApproachingWall::GamesPlayed);

		if (bGameHasWinner && WinnerPlayerName == LocalPlayerName)
		{
			Stats->IncrementLocalStat(WjWorldStats::ApproachingWall::Wins);
		}
		else
		{
			Stats->IncrementLocalStat(WjWorldStats::ApproachingWall::Losses);
		}

		Stats->StoreStats();
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameState: Game Result - %s"), *ResultText);
}

void AWjWorldGameStatePlay::OnRep_GameRuleClass()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	AWjWorldHUDPlay* HUD = PC->GetHUD<AWjWorldHUDPlay>();
	if (!HUD) return;

	HUD->ShowGameRuleHUDWidget(GameRuleClass);
}
