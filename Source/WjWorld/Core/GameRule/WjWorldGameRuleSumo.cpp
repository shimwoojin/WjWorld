// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameRule/WjWorldGameRuleSumo.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameData/SumoGameDataComponent.h"
#include "Core/GameData/SumoPlayerDataComponent.h"
#include "GamePlay/Sumo/SumoFloorRingActor.h"
#include "GamePlay/Sumo/SumoPowerUpActor.h"
#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"
#include "WjWorldGameplayTag.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "WjWorldLogCategories.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void UWjWorldGameRuleSumo::Initialize(AWjWorldGameModePlay* InGameMode)
{
	Super::Initialize(InGameMode);

	GameDataComponentClass = USumoGameDataComponent::StaticClass();
	PlayerDataComponentClass = USumoPlayerDataComponent::StaticClass();

	// MapOption???°ë¥¸ ?¤ì • ì¡°ì •
	if (InGameMode)
	{
		const FString& MapOpt = InGameMode->GetMapOption();
		if (MapOpt.Equals(TEXT("Bridge"), ESearchCase::IgnoreCase))
		{
			ShrinkInterval = 15.f;
			PowerUpSpawnInterval = 6.f;
		}
		else if (MapOpt.Equals(TEXT("Obstacle"), ESearchCase::IgnoreCase))
		{
			ShrinkInterval = 12.f;
			PowerUpSpawnInterval = 10.f;
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Initialized (ShrinkInterval=%.1f, PowerUpInterval=%.1f)"),
		ShrinkInterval, PowerUpSpawnInterval);
}

void UWjWorldGameRuleSumo::OnGameReady()
{
	if (!HasAuthority()) return;

	Super::OnGameReady();

	// ë°”ë‹¥ ë§??˜ì§‘
	CollectFloorRings();

	// GameData???¼ìš´???•ë³´ ?¤ì •
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
		if (GameData)
		{
			GameData->SetMaxRounds(MaxRounds);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Game Ready - Players: %d, FloorRings: %d"),
		TotalPlayerCount, FloorRings.Num());
}

void UWjWorldGameRuleSumo::OnGameStart()
{
	if (!HasAuthority()) return;

	Super::OnGameStart();

	bIsGameStarted = true;
	CurrentRound = 1;
	bIsRoundActive = true;

	// GameData ?¼ìš´???…ë°?´íŠ¸
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
		if (GameData)
		{
			GameData->SetCurrentRound(CurrentRound);
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Game Started - Round %d/%d, Players: %d (Min: %d)"),
		CurrentRound, MaxRounds, TotalPlayerCount, MinimumPlayerCount);

	// ?£ì? ì¼€?´ìŠ¤: ?Œë ˆ?´ì–´ ?†ìŒ
	if (TotalPlayerCount <= 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("GameRuleSumo: No players, ending game"));
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// ?£ì? ì¼€?´ìŠ¤: ?”ë¡œ ?Œë ˆ?´ì–´
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

	// ?€?´ë¨¸ ?•ë¦¬
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundResetTimerHandle);
	}

	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		if (WinnerPlayer.IsValid())
		{
			GameState->SetGameResult(WinnerPlayer->GetPlayerName(), true);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Final Winner: %s"), *WinnerPlayer->GetPlayerName());
		}
		else
		{
			GameState->SetGameResult(TEXT(""), false);
			UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: No winner (Draw)"));
		}
	}

	// ?Œì›Œ???•ë¦¬
	CleanupPowerUps();

	Super::OnGameEnd();
}

void UWjWorldGameRuleSumo::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerJoined(Player);

	if (!HasAuthority() || !Player) return;

	AlivePlayers.Add(Player);
	AllPlayers.Add(Player);
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

	// ìºë¦­???œê±° ì²˜ë¦¬
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
	AllPlayers.Remove(Player);
	TotalPlayerCount--;

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player Left - %s, Alive: %d, Total: %d"),
		*Player->GetPlayerName(), AlivePlayerCount, TotalPlayerCount);

	// ëª¨ë“  ?Œë ˆ?´ì–´ ?´íƒˆ
	if (TotalPlayerCount <= 0)
	{
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// ?¼ìš´???¹ë¦¬ ì¡°ê±´ ì²´í¬
	if (bIsGameStarted && bIsRoundActive && AlivePlayerCount <= 1)
	{
		OnRoundEnd();
	}
}

void UWjWorldGameRuleSumo::OnPlayerEliminated(AWjWorldCharacterPlay* EliminatedCharacter)
{
	if (!HasAuthority() || !EliminatedCharacter) return;
	if (EliminatedCharacter->IsEliminated()) return;

	AWjWorldPlayerStatePlay* VictimPS = EliminatedCharacter->GetPlayerState<AWjWorldPlayerStatePlay>();
	FString VictimName = VictimPS ? VictimPS->GetPlayerName() : TEXT("Unknown");

	// ???¤íƒ¯ ê¸°ë¡ + ?¬í”¼??
	AWjWorldCharacterPlay* Attacker = EliminatedCharacter->GetLastAttacker();
	FString KillerName;
	if (Attacker && !Attacker->IsEliminated())
	{
		AWjWorldPlayerStatePlay* AttackerPS = Attacker->GetPlayerState<AWjWorldPlayerStatePlay>();
		if (AttackerPS)
		{
			KillerName = AttackerPS->GetPlayerName();
			RecordKillStat(AttackerPS);
		}
	}

	// ?¬í”¼??ë¸Œë¡œ?œìº?¤íŠ¸
	BroadcastKillFeed(KillerName, VictimName);

	// ìºë¦­???œê±°
	EliminatedCharacter->OnEliminated();

	// PlayerData ?…ë°?´íŠ¸
	if (VictimPS)
	{
		if (USumoPlayerDataComponent* PlayerData = VictimPS->GetGameData<USumoPlayerDataComponent>())
		{
			PlayerData->OnEliminated();
		}

		// ?ˆë½ ?œì„œ ê¸°ë¡
		EliminationOrder.Add(VictimPS);

		if (AlivePlayers.Remove(VictimPS) > 0)
		{
			AlivePlayerCount--;
		}

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player eliminated: %s. Alive: %d (Round %d)"),
			*VictimPS->GetPlayerName(), AlivePlayerCount, CurrentRound);
	}

	UpdateGameData();

	// ?™ì‹œ ?„ì› ?ˆë½
	if (AlivePlayerCount == 0)
	{
		OnRoundEnd();
		return;
	}

	// ?¼ìš´???¹ë¦¬ ì¡°ê±´ (1ëª??´í•˜ ?ì¡´)
	if (bIsGameStarted && bIsRoundActive && AlivePlayerCount <= 1)
	{
		OnRoundEnd();
	}
}

void UWjWorldGameRuleSumo::BroadcastKillFeed(const FString& KillerName, const FString& VictimName)
{
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (!GameState) return;

	USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
	if (!GameData) return;

	FString FeedText;
	if (KillerName.IsEmpty())
	{
		FeedText = FString::Printf(TEXT("%s fell off!"), *VictimName);
	}
	else
	{
		FeedText = FString::Printf(TEXT("%s eliminated %s"), *KillerName, *VictimName);
	}

	GameData->SetKillFeedText(FeedText);
}

// --- ?¼ìš´???œìŠ¤??---

void UWjWorldGameRuleSumo::OnRoundEnd()
{
	if (!bIsRoundActive) return;
	bIsRoundActive = false;

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Round %d ended. Alive: %d"), CurrentRound, AlivePlayerCount);

	// ?ì¡´?ë„ ?ˆë½ ?œì„œ??ì¶”ê? (?¼ìš´???°ìŠ¹)
	if (AlivePlayerCount == 1)
	{
		for (const auto& AlivePlayer : AlivePlayers)
		{
			if (AlivePlayer.IsValid())
			{
				EliminationOrder.Add(AlivePlayer);
				break;
			}
		}
	}

	// ?œìœ„ ?ìˆ˜ ë¶€??
	AwardRoundScores();
	UpdatePlayerScores();

	// ìµœì¢… ?¼ìš´?œì¸ì§€ ì²´í¬
	if (CurrentRound >= MaxRounds)
	{
		// ìµœì¢… ?¹ì ê²°ì • (ì´ì  ê¸°ì?)
		WinnerPlayer = GetFinalWinner();
		bGameOverConditionMet = true;
		OnGameEnd();
		return;
	}

	// ?¤ìŒ ?¼ìš´??ì¤€ë¹?
	bIsResettingRound = true;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(RoundResetTimerHandle, FTimerDelegate::CreateUObject(this, &UWjWorldGameRuleSumo::ResetRound),
			RoundResetDelay, false);
	}
}

void UWjWorldGameRuleSumo::AwardRoundScores()
{
	// ?ˆë½ ?œì„œ ??ˆœ?¼ë¡œ ?ìˆ˜ ë¶€??
	// ì²?ë²ˆì§¸ ?ˆë½ = 1?? ??ë²ˆì§¸ = 2?? ... ë§ˆì?ë§??ì¡´ = N??
	int32 NumPlayers = EliminationOrder.Num();
	for (int32 i = 0; i < NumPlayers; i++)
	{
		if (EliminationOrder[i].IsValid())
		{
			int32 Score = i + 1; // ì²??ˆë½??1?? ë§ˆì?ë§??ì¡´??N??
			AWjWorldPlayerStatePlay* PS = EliminationOrder[i].Get();
			if (USumoPlayerDataComponent* PlayerData = PS->GetGameData<USumoPlayerDataComponent>())
			{
				PlayerData->AddScore(Score);
				UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Round score - %s: +%d (Total: %d)"),
					*PS->GetPlayerName(), Score, PlayerData->GetTotalScore());
			}
		}
	}
}

void UWjWorldGameRuleSumo::UpdatePlayerScores()
{
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (!GameState) return;

	USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
	if (!GameData) return;

	TArray<FSumoPlayerScore> Scores;
	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;
		AWjWorldPlayerStatePlay* PS = WeakPS.Get();
		USumoPlayerDataComponent* PlayerData = PS->GetGameData<USumoPlayerDataComponent>();

		FSumoPlayerScore Entry;
		Entry.PlayerName = PS->GetPlayerName();
		Entry.Score = PlayerData ? PlayerData->GetTotalScore() : 0;
		Scores.Add(Entry);
	}

	// ?ìˆ˜ ?´ë¦¼ì°¨ìˆœ ?•ë ¬
	Scores.Sort([](const FSumoPlayerScore& A, const FSumoPlayerScore& B) { return A.Score > B.Score; });
	GameData->SetPlayerScores(Scores);
}

AWjWorldPlayerStatePlay* UWjWorldGameRuleSumo::GetFinalWinner() const
{
	AWjWorldPlayerStatePlay* BestPlayer = nullptr;
	int32 BestScore = -1;

	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;
		AWjWorldPlayerStatePlay* PS = WeakPS.Get();
		USumoPlayerDataComponent* PlayerData = PS->GetGameData<USumoPlayerDataComponent>();
		int32 Score = PlayerData ? PlayerData->GetTotalScore() : 0;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestPlayer = PS;
		}
	}

	return BestPlayer;
}

void UWjWorldGameRuleSumo::ResetRound()
{
	bIsResettingRound = false;

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Resetting for round %d"), CurrentRound + 1);

	UWorld* World = GetWorld();
	if (!World) return;

	// ?Œì›Œ???•ë¦¬
	CleanupPowerUps();

	// ë²„í”„ ?•ë¦¬
	RemoveAllPlayerBuffs();

	// ?Œë«??ë³µì›
	for (ASumoFloorRingActor* Ring : FloorRings)
	{
		if (Ring)
		{
			Ring->ResetRing();
		}
	}
	CurrentShrinkLevel = 0;
	TimeSinceLastShrink = 0.f;
	bIsWarningActive = false;
	WarningElapsed = 0.f;
	TimeSinceLastPowerUp = 0.f;

	// ?ˆë½ ?œì„œ ì´ˆê¸°??
	EliminationOrder.Empty();

	// ëª¨ë“  ?Œë ˆ?´ì–´ ë¦¬ìŠ¤??
	AlivePlayers.Empty();
	AlivePlayerCount = 0;

	AWjWorldGameModePlay* GM = GetGameModePlay();
	if (!GM) return;

	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;
		AWjWorldPlayerStatePlay* PS = WeakPS.Get();

		// PlayerData ë¦¬ì…‹
		if (USumoPlayerDataComponent* PlayerData = PS->GetGameData<USumoPlayerDataComponent>())
		{
			PlayerData->ResetForNewRound();
		}

		AlivePlayers.Add(PS);
		AlivePlayerCount++;

		// ìºë¦­??ë¦¬ìŠ¤??
		APlayerController* PC = Cast<APlayerController>(PS->GetOwner());
		if (PC)
		{
			// ê¸°ì¡´ Pawn ?Œê´´
			if (APawn* OldPawn = PC->GetPawn())
			{
				OldPawn->Destroy();
			}
			GM->RestartPlayer(PC);
		}
	}

	// ?¼ìš´???œì‘
	OnRoundStart();
}

void UWjWorldGameRuleSumo::OnRoundStart()
{
	CurrentRound++;
	bIsRoundActive = true;

	// GameData ?…ë°?´íŠ¸
	AWjWorldGameStatePlay* GameState = GetGameStatePlay();
	if (GameState)
	{
		USumoGameDataComponent* GameData = GameState->GetGameData<USumoGameDataComponent>();
		if (GameData)
		{
			GameData->SetCurrentRound(CurrentRound);
		}
	}

	UpdateGameData();

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Round %d/%d started. Players: %d"),
		CurrentRound, MaxRounds, AlivePlayerCount);
}

// --- ?¹ë¦¬ ì¡°ê±´ ---

bool UWjWorldGameRuleSumo::CheckWinCondition() const
{
	if (!bIsGameStarted) return false;
	return bGameOverConditionMet;
}

AWjWorldPlayerStatePlay* UWjWorldGameRuleSumo::GetWinner() const
{
	return WinnerPlayer.Get();
}

// --- ??---

void UWjWorldGameRuleSumo::TickGameRule(float DeltaTime)
{
	if (!HasAuthority()) return;
	if (!bIsGameStarted || bGameOverConditionMet || bIsResettingRound) return;

	if (bIsRoundActive)
	{
		CheckFallenPlayers();
		TickShrinkPlatform(DeltaTime);
		TickPowerUpSpawn(DeltaTime);
	}
}

void UWjWorldGameRuleSumo::CheckFallenPlayers()
{
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

// --- ì¶•ì†Œ ?Œë«??---

void UWjWorldGameRuleSumo::CollectFloorRings()
{
	FloorRings.Empty();

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<ASumoFloorRingActor> It(World); It; ++It)
	{
		FloorRings.Add(*It);
	}

	// RingOrder ?´ë¦¼ì°¨ìˆœ ?•ë ¬ (?¸ê³½ë¶€???Œê´´)
	FloorRings.Sort([](const ASumoFloorRingActor& A, const ASumoFloorRingActor& B)
	{
		return A.RingOrder > B.RingOrder;
	});

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Collected %d floor rings"), FloorRings.Num());
}

void UWjWorldGameRuleSumo::TickShrinkPlatform(float DeltaTime)
{
	if (FloorRings.Num() == 0) return;

	// Warning ì¤‘ì´ë©?ê²½ê³  ?œê°„ ì²´í¬
	if (bIsWarningActive)
	{
		WarningElapsed += DeltaTime;
		if (WarningElapsed >= WarningDuration)
		{
			// Warning ?„ë£Œ ??ë§??Œê´´
			for (ASumoFloorRingActor* Ring : FloorRings)
			{
				if (Ring && Ring->GetRingState() == ESumoRingState::Warning)
				{
					Ring->DestroyRing();
				}
			}
			bIsWarningActive = false;
			WarningElapsed = 0.f;
			CurrentShrinkLevel++;
		}
		return;
	}

	TimeSinceLastShrink += DeltaTime;
	if (TimeSinceLastShrink >= ShrinkInterval)
	{
		TimeSinceLastShrink = 0.f;

		// ?¤ìŒ ?Œê´´ ?€??ë§?ì°¾ê¸°
		ASumoFloorRingActor* TargetRing = nullptr;
		for (ASumoFloorRingActor* Ring : FloorRings)
		{
			if (Ring && Ring->GetRingState() == ESumoRingState::Active && Ring->RingOrder > 0)
			{
				TargetRing = Ring;
				break; // ?´ë? ?´ë¦¼ì°¨ìˆœ?´ë?ë¡?ì²?Activeê°€ ìµœì™¸ê³?
			}
		}

		if (TargetRing)
		{
			// ê°™ì? RingOrder??ëª¨ë“  ë§ì— Warning ?œì‘
			int32 TargetOrder = TargetRing->RingOrder;
			for (ASumoFloorRingActor* Ring : FloorRings)
			{
				if (Ring && Ring->RingOrder == TargetOrder && Ring->GetRingState() == ESumoRingState::Active)
				{
					Ring->StartWarning();
				}
			}

			bIsWarningActive = true;
			WarningElapsed = 0.f;

			UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Shrink warning - RingOrder %d"), TargetOrder);
		}
	}
}

// --- ?Œì›Œ??---

void UWjWorldGameRuleSumo::TickPowerUpSpawn(float DeltaTime)
{
	if (!PowerUpActorClass) return;

	TimeSinceLastPowerUp += DeltaTime;
	if (TimeSinceLastPowerUp >= PowerUpSpawnInterval)
	{
		TimeSinceLastPowerUp = 0.f;

		// ?Œë©¸???Œì›Œ???•ë¦¬
		ActivePowerUps.RemoveAll([](const TWeakObjectPtr<ASumoPowerUpActor>& Ptr) { return !Ptr.IsValid(); });

		if (ActivePowerUps.Num() < MaxActivePowerUps)
		{
			SpawnPowerUp();
		}
	}
}

void UWjWorldGameRuleSumo::SpawnPowerUp()
{
	UWorld* World = GetWorld();
	if (!World || !PowerUpActorClass) return;

	FVector SpawnLoc = GetRandomSpawnLocationOnActiveRings();
	SpawnLoc.Z += 100.f; // ?Œë«???„ì— ?„ìš°ê¸?

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASumoPowerUpActor* PowerUp = World->SpawnActor<ASumoPowerUpActor>(PowerUpActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (PowerUp)
	{
		// ?œë¤ ?€???¤ì •
		int32 RandType = FMath::RandRange(0, 2);
		PowerUp->PowerUpType = static_cast<ESumoPowerUpType>(RandType);

		ActivePowerUps.Add(PowerUp);

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Spawned PowerUp type %d at %s"), RandType, *SpawnLoc.ToString());
	}
}

FVector UWjWorldGameRuleSumo::GetRandomSpawnLocationOnActiveRings() const
{
	// Active ?íƒœ??ë§?ì¤??˜ë‚˜ë¥??œë¤ ? íƒ
	TArray<ASumoFloorRingActor*> ActiveRings;
	for (ASumoFloorRingActor* Ring : FloorRings)
	{
		if (Ring && Ring->GetRingState() == ESumoRingState::Active)
		{
			ActiveRings.Add(Ring);
		}
	}

	if (ActiveRings.Num() > 0)
	{
		ASumoFloorRingActor* ChosenRing = ActiveRings[FMath::RandRange(0, ActiveRings.Num() - 1)];
		FVector Center = ChosenRing->GetActorLocation();
		float Radius = ChosenRing->RingRadius;

		// ë§????œë¤ ?„ì¹˜
		float Angle = FMath::FRandRange(0.f, 2.f * PI);
		float Dist = FMath::FRandRange(0.f, Radius);
		return Center + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
	}

	return FVector::ZeroVector;
}

void UWjWorldGameRuleSumo::CleanupPowerUps()
{
	for (const auto& WeakPU : ActivePowerUps)
	{
		if (WeakPU.IsValid())
		{
			WeakPU->Destroy();
		}
	}
	ActivePowerUps.Empty();
}

void UWjWorldGameRuleSumo::RemoveAllPlayerBuffs()
{
	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;

		APawn* Pawn = WeakPS->GetPawn();
		AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(Pawn);
		if (!Character) continue;

		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (!ASC) continue;

		// ëª¨ë“  ë²„í”„ ?œê·¸ ?œê±°
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_SpeedBoost());
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_SuperPush());
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_Shield());

		// ?´ë™?ë„ ë³µì› (SpeedBoostê°€ ?œì„± ì¤‘ì´?ˆì„ ???ˆìŒ)
		if (UCharacterMovementComponent* MC = Character->GetCharacterMovement())
		{
			// ê¸°ë³¸ê°’ìœ¼ë¡?ë³µì› (CDO?ì„œ)
			MC->MaxWalkSpeed = Character->GetClass()->GetDefaultObject<ACharacter>()->GetCharacterMovement()->MaxWalkSpeed;
		}
	}
}


