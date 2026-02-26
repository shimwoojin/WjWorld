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

	// 諛붾떏 留??섏쭛
	CollectFloorRings();

	// GameData???쇱슫???뺣낫 ?ㅼ젙
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

	// GameData ?쇱슫???낅뜲?댄듃
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

	// ?ｌ? 耳?댁뒪: ?뚮젅?댁뼱 ?놁쓬
	if (TotalPlayerCount <= 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("GameRuleSumo: No players, ending game"));
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// ?ｌ? 耳?댁뒪: ?붾줈 ?뚮젅?댁뼱
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

	// ??대㉧ ?뺣━
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

	// ?뚯썙???뺣━
	CleanupPowerUps();

	Super::OnGameEnd();
}

void UWjWorldGameRuleSumo::OnPlayerJoined(AWjWorldPlayerStatePlay* Player)
{
	Super::OnPlayerJoined(Player);

	if (!HasAuthority() || !Player) return;

	// 게임 진행 중 입장한 관전자는 참여자로 등록하지 않음
	if (IsGameInProgress())
	{
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Spectator joined mid-game: %s"), *Player->GetPlayerName());
		return;
	}

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

	// 관전자(AllPlayers에 없는 플레이어)는 게임 카운터에 영향 없음
	if (AllPlayers.Remove(Player) == 0)
	{
		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Spectator left: %s"), *Player->GetPlayerName());
		return;
	}

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

	// 모든 참여자 이탈
	if (TotalPlayerCount <= 0)
	{
		bGameOverConditionMet = true;
		WinnerPlayer = nullptr;
		OnGameEnd();
		return;
	}

	// 승리 조건 체크
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

	// ???ㅽ꺈 湲곕줉 + ?ы뵾??
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

	// ?ы뵾??釉뚮줈?쒖틦?ㅽ듃
	BroadcastKillFeed(KillerName, VictimName);

	// 罹먮┃???쒓굅
	EliminatedCharacter->OnEliminated();

	// PlayerData ?낅뜲?댄듃
	if (VictimPS)
	{
		if (USumoPlayerDataComponent* PlayerData = VictimPS->GetGameData<USumoPlayerDataComponent>())
		{
			PlayerData->OnEliminated();
		}

		// ?덈씫 ?쒖꽌 湲곕줉
		EliminationOrder.Add(VictimPS);

		if (AlivePlayers.Remove(VictimPS) > 0)
		{
			AlivePlayerCount--;
		}

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player eliminated: %s. Alive: %d (Round %d)"),
			*VictimPS->GetPlayerName(), AlivePlayerCount, CurrentRound);
	}

	UpdateGameData();

	// ?숈떆 ?꾩썝 ?덈씫
	if (AlivePlayerCount == 0)
	{
		OnRoundEnd();
		return;
	}

	// ?쇱슫???밸━ 議곌굔 (1紐??댄븯 ?앹〈)
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

// --- ?쇱슫???쒖뒪??---

void UWjWorldGameRuleSumo::OnRoundEnd()
{
	if (!bIsRoundActive) return;
	bIsRoundActive = false;

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Round %d ended. Alive: %d"), CurrentRound, AlivePlayerCount);

	// ?앹〈?먮룄 ?덈씫 ?쒖꽌??異붽? (?쇱슫???곗듅)
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

	// ?쒖쐞 ?먯닔 遺??
	AwardRoundScores();
	UpdatePlayerScores();

	// 理쒖쥌 ?쇱슫?쒖씤吏 泥댄겕
	if (CurrentRound >= MaxRounds)
	{
		// 理쒖쥌 ?뱀옄 寃곗젙 (珥앹젏 湲곗?)
		WinnerPlayer = GetFinalWinner();
		bGameOverConditionMet = true;
		OnGameEnd();
		return;
	}

	// ?ㅼ쓬 ?쇱슫??以鍮?
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
	// ?덈씫 ?쒖꽌 ??닚?쇰줈 ?먯닔 遺??
	// 泥?踰덉㎏ ?덈씫 = 1?? ??踰덉㎏ = 2?? ... 留덉?留??앹〈 = N??
	int32 NumPlayers = EliminationOrder.Num();
	for (int32 i = 0; i < NumPlayers; i++)
	{
		if (EliminationOrder[i].IsValid())
		{
			int32 Score = i + 1; // 泥??덈씫??1?? 留덉?留??앹〈??N??
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

	// ?먯닔 ?대┝李⑥닚 ?뺣젹
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

	// ?뚯썙???뺣━
	CleanupPowerUps();

	// 踰꾪봽 ?뺣━
	RemoveAllPlayerBuffs();

	// ?뚮옯??蹂듭썝
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

	// ?덈씫 ?쒖꽌 珥덇린??
	EliminationOrder.Empty();

	// 紐⑤뱺 ?뚮젅?댁뼱 由ъ뒪??
	AlivePlayers.Empty();
	AlivePlayerCount = 0;

	AWjWorldGameModePlay* GM = GetGameModePlay();
	if (!GM) return;

	for (const auto& WeakPS : AllPlayers)
	{
		if (!WeakPS.IsValid()) continue;
		AWjWorldPlayerStatePlay* PS = WeakPS.Get();

		// PlayerData 由ъ뀑
		if (USumoPlayerDataComponent* PlayerData = PS->GetGameData<USumoPlayerDataComponent>())
		{
			PlayerData->ResetForNewRound();
		}

		AlivePlayers.Add(PS);
		AlivePlayerCount++;

		// 罹먮┃??由ъ뒪??
		APlayerController* PC = Cast<APlayerController>(PS->GetOwner());
		if (PC)
		{
			// 湲곗〈 Pawn ?뚭눼
			// 기존 Pawn 제거 (이미 Destroy된 경우 무시)
			if (APawn* OldPawn = PC->GetPawn())
			{
				if (::IsValid(OldPawn))
				{
					OldPawn->Destroy();
				}
			}

			// UnPossess 후 RestartPlayer (Pawn 없이 리스폰 보장)
			PC->UnPossess();
			GM->RestartPlayer(PC);

			// 리스폰 성공 확인
			if (APawn* NewPawn = PC->GetPawn())
			{
				UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Player %s respawned successfully"), *PS->GetPlayerName());
			}
			else
			{
				UE_LOG(LogWjWorld, Warning, TEXT("GameRuleSumo: Player %s respawn FAILED"), *PS->GetPlayerName());
			}
		}
	}

	// ?쇱슫???쒖옉
	OnRoundStart();
}

void UWjWorldGameRuleSumo::OnRoundStart()
{
	CurrentRound++;
	bIsRoundActive = true;

	// GameData ?낅뜲?댄듃
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

// --- ?밸━ 議곌굔 ---

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

// --- 異뺤냼 ?뚮옯??---

void UWjWorldGameRuleSumo::CollectFloorRings()
{
	FloorRings.Empty();

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<ASumoFloorRingActor> It(World); It; ++It)
	{
		FloorRings.Add(*It);
	}

	// RingOrder ?대┝李⑥닚 ?뺣젹 (?멸낸遺???뚭눼)
	FloorRings.Sort([](const ASumoFloorRingActor& A, const ASumoFloorRingActor& B)
	{
		return A.RingOrder > B.RingOrder;
	});

	UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Collected %d floor rings"), FloorRings.Num());
}

void UWjWorldGameRuleSumo::TickShrinkPlatform(float DeltaTime)
{
	if (FloorRings.Num() == 0) return;

	// Warning 以묒씠硫?寃쎄퀬 ?쒓컙 泥댄겕
	if (bIsWarningActive)
	{
		WarningElapsed += DeltaTime;
		if (WarningElapsed >= WarningDuration)
		{
			// Warning ?꾨즺 ??留??뚭눼
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

		// ?ㅼ쓬 ?뚭눼 ???留?李얘린
		ASumoFloorRingActor* TargetRing = nullptr;
		for (ASumoFloorRingActor* Ring : FloorRings)
		{
			if (Ring && Ring->GetRingState() == ESumoRingState::Active && Ring->RingOrder > 0)
			{
				TargetRing = Ring;
				break; // ?대? ?대┝李⑥닚?대?濡?泥?Active媛 理쒖쇅怨?
			}
		}

		if (TargetRing)
		{
			// 媛숈? RingOrder??紐⑤뱺 留곸뿉 Warning ?쒖옉
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

// --- ?뚯썙??---

void UWjWorldGameRuleSumo::TickPowerUpSpawn(float DeltaTime)
{
	if (!PowerUpActorClass) return;

	TimeSinceLastPowerUp += DeltaTime;
	if (TimeSinceLastPowerUp >= PowerUpSpawnInterval)
	{
		TimeSinceLastPowerUp = 0.f;

		// ?뚮㈇???뚯썙???뺣━
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
	SpawnLoc.Z += 100.f; // ?뚮옯???꾩뿉 ?꾩슦湲?

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASumoPowerUpActor* PowerUp = World->SpawnActor<ASumoPowerUpActor>(PowerUpActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (PowerUp)
	{
		// ?쒕뜡 ????ㅼ젙
		int32 RandType = FMath::RandRange(0, 2);
		PowerUp->SetPowerUpType(static_cast<ESumoPowerUpType>(RandType));

		ActivePowerUps.Add(PowerUp);

		UE_LOG(LogWjWorld, Log, TEXT("GameRuleSumo: Spawned PowerUp type %d at %s"), RandType, *SpawnLoc.ToString());
	}
}

FVector UWjWorldGameRuleSumo::GetRandomSpawnLocationOnActiveRings() const
{
	// Active ?곹깭??留?以??섎굹瑜??쒕뜡 ?좏깮
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

		// 留????쒕뜡 ?꾩튂
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
		AWjWorldPlayerStatePlay* PS = WeakPS.Get();

		// PlayerState에서 ASC 가져오기 (ASC는 PlayerState에 있음)
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		if (!ASC) continue;

		// 紐⑤뱺 踰꾪봽 ?쒓렇 ?쒓굅
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_SpeedBoost());
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_SuperPush());
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_Shield());

		// State_Eliminated 태그 제거 (다음 라운드 어빌리티 활성화를 위해)
		ASC->RemoveLooseGameplayTag(WjWorldGameplayTag::State_Eliminated());

		// 이동속도 복원 (기존 캐릭터가 존재하는 경우에만)
		APawn* Pawn = PS->GetPawn();
		AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(Pawn);
		if (Character)
		{
			if (UCharacterMovementComponent* MC = Character->GetCharacterMovement())
			{
				// 기본값으로 복원 (CDO에서)
				MC->MaxWalkSpeed = Character->GetClass()->GetDefaultObject<ACharacter>()->GetCharacterMovement()->MaxWalkSpeed;
			}
		}
	}
}



