// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleSumo.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameData/SumoGameDataComponent.h"
#include "Core/GameData/SumoPlayerDataComponent.h"
#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"

#include "WjWorldLogCategories.h"

void UWjWorldGameRuleSumo::Initialize(AWjWorldGameModePlay* InGameMode)
{
	Super::Initialize(InGameMode);

	GameDataComponentClass = USumoGameDataComponent::StaticClass();
	PlayerDataComponentClass = USumoPlayerDataComponent::StaticClass();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Initialized"));
}

void UWjWorldGameRuleSumo::OnGameReady()
{
	if (!HasAuthority()) return;

	Super::OnGameReady();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Game Ready - Players: %d"), TotalPlayerCount);
}

void UWjWorldGameRuleSumo::OnGameStart()
{
	if (!HasAuthority()) return;

	Super::OnGameStart();

	bIsGameStarted = true;

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Game Started - Players: %d (Min: %d)"),
		TotalPlayerCount, MinimumPlayerCount);

	// 엣지 케이스: 플레이어 없음
	if (TotalPlayerCount <= 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("GameRuleSumo: No players, ending game"));
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// 엣지 케이스: 솔로 플레이어
	if (TotalPlayerCount == 1 && AlivePlayerCount == 1)
	{
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Solo player wins by default"));
		if (AlivePlayers.Num() > 0 && AlivePlayers[0].IsValid())
		{
			WinnerPlayer = AlivePlayers[0];
		}
		bGameOverConditionMet = true;
		OnGameEnd();
		return;
	}
}

void UWjWorldGameRuleSumo::OnGameEnd()
{
	if (!HasAuthority()) return;

	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		if (WinnerPlayer.IsValid())
		{
			GameState->SetGameResult(WinnerPlayer->GetPlayerName(), true);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Winner: %s"), *WinnerPlayer->GetPlayerName());
		}
		else
		{
			GameState->SetGameResult(TEXT(""), false);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: No winner (Draw)"));
		}
	}

	Super::OnGameEnd();
}

void UWjWorldGameRuleSumo::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerJoined(Player);

	if (!HasAuthority() || !Player) return;

	AlivePlayers.Add(Player);
	AlivePlayerCount++;
	TotalPlayerCount++;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player Joined - %s, Alive: %d, Total: %d"),
		*Player->GetPlayerName(), AlivePlayerCount, TotalPlayerCount);
}

void UWjWorldGameRuleSumo::OnPlayerLeft(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerLeft(Player);

	if (!HasAuthority() || !Player) return;

	// 캐릭터 제거 처리
	APawn* PlayerPawn = Player->GetPawn();
	AWjWorldCharacterPlay* PlayerCharacter = Cast<AWjWorldCharacterPlay>(PlayerPawn);
	if (PlayerCharacter && !PlayerCharacter->IsEliminated())
	{
		PlayerCharacter->OnEliminated();
	}

	if (AlivePlayers.Remove(Player) > 0)
	{
		AlivePlayerCount--;
	}
	TotalPlayerCount--;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player Left - %s, Alive: %d, Total: %d"),
		*Player->GetPlayerName(), AlivePlayerCount, TotalPlayerCount);

	// 모든 플레이어 이탈
	if (TotalPlayerCount <= 0)
	{
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// 승리 조건 체크
	if (bIsGameStarted && CheckWinCondition())
	{
		if (AlivePlayerCount == 1 && AlivePlayers.Num() > 0)
		{
			for (const auto& AlivePlayer : AlivePlayers)
			{
				if (AlivePlayer.IsValid())
				{
					WinnerPlayer = AlivePlayer;
					break;
				}
			}
		}
		bGameOverConditionMet = true;
		OnGameEnd();
	}
}

void UWjWorldGameRuleSumo::OnPlayerEliminated(AWjWorldCharacterPlay* EliminatedCharacter)
{
	if (!HasAuthority() || !EliminatedCharacter) return;
	if (EliminatedCharacter->IsEliminated()) return;

	// 킬 스탯 기록
	AWjWorldCharacterPlay* Attacker = EliminatedCharacter->GetLastAttacker();
	if (Attacker && !Attacker->IsEliminated())
	{
		AWjWorldPlayerStatePlay* AttackerPS = Attacker->GetPlayerState<AWjWorldPlayerStatePlay>();
		if (AttackerPS)
		{
			RecordKillStat(AttackerPS);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Kill recorded for %s"),
				*AttackerPS->GetPlayerName());
		}
	}

	// 캐릭터 제거
	EliminatedCharacter->OnEliminated();

	AWjWorldPlayerStatePlay* PlayerState = EliminatedCharacter->GetPlayerState<AWjWorldPlayerStatePlay>();
	if (PlayerState)
	{
		if (AlivePlayers.Remove(PlayerState) > 0)
		{
			AlivePlayerCount--;
		}

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player eliminated: %s. Alive: %d"),
			*PlayerState->GetPlayerName(), AlivePlayerCount);
	}

	UpdateGameData();

	// 동시 전원 탈락
	if (AlivePlayerCount == 0)
	{
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// 승리 조건 체크
	if (bIsGameStarted && CheckWinCondition())
	{
		bGameOverConditionMet = true;

		if (AlivePlayerCount == 1 && AlivePlayers.Num() > 0)
		{
			for (const auto& AlivePlayer : AlivePlayers)
			{
				if (AlivePlayer.IsValid())
				{
					WinnerPlayer = AlivePlayer;
					break;
				}
			}
		}

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Win condition met! Winner: %s"),
			WinnerPlayer.IsValid() ? *WinnerPlayer->GetPlayerName() : TEXT("None"));

		OnGameEnd();
	}
}

bool UWjWorldGameRuleSumo::CheckWinCondition() const
{
	if (!bIsGameStarted) return false;
	return AlivePlayerCount <= 1;
}

AWjWorldPlayerStatePlay* UWjWorldGameRuleSumo::GetWinner() const
{
	return WinnerPlayer.Get();
}

void UWjWorldGameRuleSumo::TickGameRule(float DeltaTime)
{
	if (!HasAuthority()) return;
	if (!bIsGameStarted || bGameOverConditionMet) return;

	CheckFallenPlayers();
}

void UWjWorldGameRuleSumo::CheckFallenPlayers()
{
	// 살아있는 플레이어 중 Z 위치가 FallThresholdZ 미만인 플레이어 제거
	// 순회 중 배열 변경을 방지하기 위해 제거 대상을 먼저 수집
	TArray<AWjWorldCharacterPlay*> FallenCharacters;

	for (const auto& WeakPS : AlivePlayers)
	{
		if (!WeakPS.IsValid()) continue;

		APawn* Pawn = WeakPS->GetPawn();
		AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(Pawn);
		if (!Character || Character->IsEliminated()) continue;

		if (Character->GetActorLocation().Z < FallThresholdZ)
		{
			FallenCharacters.Add(Character);
		}
	}

	for (AWjWorldCharacterPlay* Fallen : FallenCharacters)
	{
		OnPlayerEliminated(Fallen);
	}
}

void UWjWorldGameRuleSumo::UpdateGameData()
{
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (!GameState) return;

	USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
	if (!GameData) return;

	GameData->SetAlivePlayerCount(AlivePlayerCount);
	GameData->SetTotalPlayerCount(TotalPlayerCount);
}

void UWjWorldGameRuleSumo::RecordKillStat(AWjWorldPlayerStatePlay* KillerPlayerState)
{
	if (!KillerPlayerState) return;
	if (!HasAuthority()) return;

	APlayerController* KillerPC = Cast<APlayerController>(KillerPlayerState->GetOwner());
	if (KillerPC && KillerPC->IsLocalController())
	{
		UWjWorldStatsSubsystem* Stats = GetWorld()->GetGameInstance()->GetSubsystem<UWjWorldStatsSubsystem>();
		if (Stats)
		{
			Stats->IncrementLocalStat(WjWorldStats::Sumo::Kills);
			Stats->StoreStats();
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Recorded kill for %s"),
		*KillerPlayerState->GetPlayerName());
}
