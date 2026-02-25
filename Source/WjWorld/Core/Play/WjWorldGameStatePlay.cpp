// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/GameData/WjWorldGameDataComponent.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"
#include "Currency/WjWorldCurrencySubsystem.h"
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
	DOREPLIFETIME(AWjWorldGameStatePlay, CountdownStartServerTime);
	DOREPLIFETIME(AWjWorldGameStatePlay, bIsGameEndCountDownReady);
	DOREPLIFETIME(AWjWorldGameStatePlay, EndCountDownTime);
	DOREPLIFETIME(AWjWorldGameStatePlay, WinnerPlayerName);
	DOREPLIFETIME(AWjWorldGameStatePlay, bGameHasWinner);
	DOREPLIFETIME(AWjWorldGameStatePlay, bGameResultReady);
	DOREPLIFETIME(AWjWorldGameStatePlay, GameRuleClass);
	DOREPLIFETIME(AWjWorldGameStatePlay, AllowedAbilityTags);
	DOREPLIFETIME(AWjWorldGameStatePlay, StatNamespace);
	DOREPLIFETIME(AWjWorldGameStatePlay, InitialPlayerCount);
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

	// 서버 시간 기록 (늦게 참여하는 클라이언트가 남은 시간 계산 가능)
	if (HasAuthority())
	{
		CountdownStartServerTime = GetServerWorldTimeSeconds();
	}

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
		// 게임 종료 후 ServerTravel 중 호스트 퇴장 시 마이그레이션 스킵용
		if (UWjWorldGameInstance* GI = Cast<UWjWorldGameInstance>(GetGameInstance()))
		{
			GI->MarkGameEndTraveling();
		}
		break;
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameState: Phase changed to %d"), static_cast<int32>(CurrentPhase));
}

void AWjWorldGameStatePlay::OnRep_IsGameStartCountDownReady()
{
	if (bIsGameStartCountDownReady)
	{
		UWorld* World = GetWorld();
		if (!World) return;

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC) return;

		AWjWorldHUDPlay* HUD = PC->GetHUD<AWjWorldHUDPlay>();
		if (!HUD) return;

		// 늦게 참여한 클라이언트를 위해 남은 시간 계산
		float RemainingTime = StartCountDownTime;
		if (CountdownStartServerTime > 0.0)
		{
			double CurrentServerTime = GetServerWorldTimeSeconds();
			double Elapsed = CurrentServerTime - CountdownStartServerTime;
			RemainingTime = FMath::Max(0.0f, StartCountDownTime - static_cast<float>(Elapsed));
		}

		HUD->StartGameStartCountDown(RemainingTime);

		UE_LOG(LogWjWorld, Log, TEXT("GameState: Game Start Countdown Ready - %f seconds (original: %f)"), RemainingTime, StartCountDownTime);
	}
}

void AWjWorldGameStatePlay::OnRep_IsGameEndCountDownReady()
{
	if (bIsGameEndCountDownReady)
	{
		UWorld* World = GetWorld();
		if (!World) return;

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC) return;

		AWjWorldHUDPlay* HUD = PC->GetHUD<AWjWorldHUDPlay>();
		if (!HUD) return;

		HUD->StartGameStartCountDown(EndCountDownTime);

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

	// 1인 플레이 시 스탯/보상 제외
	const bool bShouldRecordStats = (InitialPlayerCount > 1);

	// 스탯 기록 (각 클라이언트가 자신의 스탯만 기록)
	// StatNamespace 기반 동적 스탯 키 생성
	UWjWorldStatsSubsystem* Stats = GetGameInstance()->GetSubsystem<UWjWorldStatsSubsystem>();
	if (bShouldRecordStats && Stats && PC->PlayerState && !StatNamespace.IsNone())
	{
		FString NS = StatNamespace.ToString();
		Stats->IncrementLocalStat(FName(*FString::Printf(TEXT("%s_GamesPlayed"), *NS)));

		if (bGameHasWinner && WinnerPlayerName == LocalPlayerName)
		{
			Stats->IncrementLocalStat(FName(*FString::Printf(TEXT("%s_Wins"), *NS)));
		}
		else
		{
			Stats->IncrementLocalStat(FName(*FString::Printf(TEXT("%s_Losses"), *NS)));
		}

		Stats->StoreStats();
	}
	else if (!bShouldRecordStats)
	{
		UE_LOG(LogWjWorld, Log, TEXT("GameState: Skipping stats/rewards for solo play (InitialPlayerCount=%d)"), InitialPlayerCount);
	}

	// 재화 보상 지급 (각 클라이언트가 자신의 보상 요청)
	UWjWorldCurrencySubsystem* CurrencySub = GetGameInstance()->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (bShouldRecordStats && CurrencySub && PC->PlayerState)
	{
		bool bIsLocalPlayerWinner = bGameHasWinner && (WinnerPlayerName == LocalPlayerName);
		CurrencySub->TriggerMatchReward(bIsLocalPlayerWinner);
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

void AWjWorldGameStatePlay::SetAllowedAbilityTags(const FGameplayTagContainer& InTags)
{
	AllowedAbilityTags = InTags;
}

void AWjWorldGameStatePlay::SetStatNamespace(FName InNamespace)
{
	StatNamespace = InNamespace;
}

void AWjWorldGameStatePlay::SetInitialPlayerCount(int32 InCount)
{
	InitialPlayerCount = InCount;
	UE_LOG(LogWjWorld, Log, TEXT("GameState: InitialPlayerCount set to %d"), InitialPlayerCount);
}
