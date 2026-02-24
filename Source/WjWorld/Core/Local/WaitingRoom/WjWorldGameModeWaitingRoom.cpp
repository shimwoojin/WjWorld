// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/Local/WaitingRoom/WjWorldHUDWaitingRoom.h"
#include "Core/Local/WaitingRoom/WjWorldGameStateWaitingRoom.h"
#include "Core/Local/WaitingRoom/WjWorldPlayerControllerWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "DataAsset/WjWorldMinigameDataAsset.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeWaitingRoom::AWjWorldGameModeWaitingRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	// ⭐ 중요: PlayerState 클래스 설정 (Base에서 설정되지만 명시적으로 재확인)
	PlayerStateClass = AWjWorldPlayerStateBase::StaticClass();

	// 기본 HUD 클래스 설정
	HUDClass = AWjWorldHUDWaitingRoom::StaticClass();

	// ⭐ WaitingRoom 전용 GameState 클래스 설정
	GameStateClass = AWjWorldGameStateWaitingRoom::StaticClass();

	// PlayerController 클래스 설정
	PlayerControllerClass = AWjWorldPlayerControllerWaitingRoom::StaticClass();

	// DefaultPawnClass는 BP 서브클래스에서 설정
}

void AWjWorldGameModeWaitingRoom::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 표시
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}

	// 배치 카탈로그 로드 (서버/클라이언트 모두)
	LoadPlacementCatalog();

	// 서버에서만 레이아웃 로드
	if (HasAuthority())
	{
		LoadHostLayoutToGameState();
	}

	// ⭐ GameState 초기화 (Server Only)
	if (HasAuthority())
	{
		UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
		if (GameInstance)
		{
			// Safety: 이전 세션 상태가 InProgress면 종료 (게임 재시작 버그 방지)
			GameInstance->EndGame();

			AWjWorldGameStateWaitingRoom* WjGameState = GetGameState<AWjWorldGameStateWaitingRoom>();

			// 마이그레이션 후 진입인지 확인
			bool bIsMigrationEntry = GameInstance->IsMigrating()
				|| GameInstance->GetMigrationState() == EHostMigrationState::Complete;

			if (bIsMigrationEntry && WjGameState)
			{
				// 마이그레이션 후 복구: 캐시된 방 설정 사용
				const FRoomSettings& CachedSettings = GameInstance->GetMigrationContext().CachedRoomSettings;
				WjGameState->InitializeRoomSettings(CachedSettings);

				UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Migration recovery - restored room settings '%s'"),
					*CachedSettings.RoomName);

				// 마이그레이션 컨텍스트 리셋
				GameInstance->GetMigrationContext().Reset();
			}
			else if (GameInstance->GetSessionManager() && WjGameState)
			{
				// 일반 진입: SessionManager에서 방 설정 가져오기
				const FRoomSettings& Settings = GameInstance->GetSessionManager()->GetLastRoomSettings();
				WjGameState->InitializeRoomSettings(Settings);

				UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: GameState initialized with room settings"));
			}
			else
			{
				UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: Failed to initialize GameState"));
			}
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: GameInstance is null"));
		}
	}

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: BeginPlay - WaitingRoom loaded"));
}

void AWjWorldGameModeWaitingRoom::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// ⭐ 디버깅: PlayerState 확인
	if (NewPlayer)
	{
		AWjWorldPlayerStateBase* PlayerState = NewPlayer->GetPlayerState<AWjWorldPlayerStateBase>();
		if (PlayerState)
		{
			UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Player joined - Name: %s, ID: %d"), 
				*PlayerState->GetPlayerName(), PlayerState->GetPlayerId());
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: PlayerState is NULL!"));
		}

		// GameState의 PlayerArray 확인
		AWjWorldGameStateWaitingRoom* WjGameState = GetGameState<AWjWorldGameStateWaitingRoom>();
		if (WjGameState)
		{
			UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Total players in GameState: %d"), 
				WjGameState->GetPlayerCount());
		}
	}
	
	// PlayerState는 GameState에서 자동으로 관리됨 (AddPlayerState)
}

void AWjWorldGameModeWaitingRoom::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Player left"));
	
	// PlayerState는 GameState에서 자동으로 관리됨 (RemovePlayerState)
}

void AWjWorldGameModeWaitingRoom::StartGame()
{
	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Starting game..."));

	// ⭐ GameState에서 현재 RoomSettings 가져오기 (UI에서 업데이트된 값)
	FRoomSettings Settings;
	AWjWorldGameStateWaitingRoom* WjGameState = GetGameState<AWjWorldGameStateWaitingRoom>();
	if (WjGameState)
	{
		Settings = WjGameState->GetRoomSettings();
	}

	// ⭐ 최소 인원 검증 (ServerTravel 전에 차단)
	const UWjWorldDeveloperSettings* DevSettings = GetDefault<UWjWorldDeveloperSettings>();
	if (DevSettings && !DevSettings->MinigameCatalog.IsNull())
	{
		UWjWorldMinigameDataAsset* Catalog = DevSettings->MinigameCatalog.LoadSynchronous();
		if (Catalog)
		{
			const FWjWorldMinigameDefinition* MinigameDef = Catalog->FindByGameModeId(FName(*Settings.GameMode));
			if (MinigameDef && WjGameState)
			{
				int32 PlayerCount = WjGameState->GetPlayerCount();
				if (PlayerCount < MinigameDef->MinimumPlayers)
				{
					UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeWaitingRoom: Not enough players (%d < %d) for %s"),
						PlayerCount, MinigameDef->MinimumPlayers, *Settings.GameMode);
					return;
				}
			}
		}
	}

	// GameInstance를 통해 세션 시작
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: GameInstance is null"));
		return;
	}

	bool bSuccess = GameInstance->StartGame();
	if (!bSuccess)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: Failed to start session"));
		return;
	}

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Session start initiated"));

	UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeWaitingRoom: Using GameState settings - GameMode: '%s', Map: '%s'"),
		*Settings.GameMode, *Settings.MapName);

	if (!WjGameState && GameInstance->GetSessionManager())
	{
		// 폴백: SessionManager에서 가져오기
		Settings = GameInstance->GetSessionManager()->GetLastRoomSettings();
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeWaitingRoom: Fallback to SessionManager settings - GameMode: '%s', Map: '%s'"),
			*Settings.GameMode, *Settings.MapName);
	}

	// 카탈로그에서 LevelPath 조회
	FString TravelURL;
	if (DevSettings && !DevSettings->MinigameCatalog.IsNull())
	{
		UWjWorldMinigameDataAsset* Catalog = DevSettings->MinigameCatalog.LoadSynchronous();
		if (Catalog)
		{
			const FWjWorldMinigameDefinition* Def = Catalog->FindByGameModeId(FName(*Settings.GameMode));
			if (Def)
			{
				// LevelPath + GameMode 명시적 지정 + GameModeId 전달 (GameRule 조회용)
				TravelURL = DevSettings->GetPlayServerTravelURL(Def->LevelPath, Settings.GameMode);

				// MapOption을 URL 옵션으로 전달
				if (!Settings.MapName.IsEmpty())
				{
					TravelURL += FString::Printf(TEXT("?MapOption=%s"), *Settings.MapName);
				}

				UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Travel to '%s' (GameModeId: %s)"),
					*TravelURL, *Settings.GameMode);
			}
			else
			{
				UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeWaitingRoom: GameModeId '%s' not found in catalog"), *Settings.GameMode);
			}
		}
	}

	// 폴백: 카탈로그에서 못 찾으면 기본 경로 사용
	if (TravelURL.IsEmpty())
	{
		TravelURL = TEXT("/Game/Map/03-1_ApproachingWall");
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeWaitingRoom: Using fallback travel URL: %s"), *TravelURL);
	}

	GetWorld()->ServerTravel(TravelURL);
}

void AWjWorldGameModeWaitingRoom::LoadPlacementCatalog()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings || Settings->PlaceableObjectCatalog.IsNull())
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeWaitingRoom: No PlaceableObjectCatalog configured"));
		return;
	}

	CachedPlacementCatalog = Settings->PlaceableObjectCatalog.LoadSynchronous();
	if (!CachedPlacementCatalog)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("WjWorldGameModeWaitingRoom: Failed to load PlaceableObjectCatalog"));
		return;
	}

	// GameState에도 카탈로그 설정 (GameStateWaitingRoom은 GameStateLobby 상속)
	AWjWorldGameStateWaitingRoom* WaitingRoomGameState = GetGameState<AWjWorldGameStateWaitingRoom>();
	if (WaitingRoomGameState)
	{
		WaitingRoomGameState->SetCatalog(CachedPlacementCatalog);
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeWaitingRoom: Placement catalog loaded"));
}

void AWjWorldGameModeWaitingRoom::LoadHostLayoutToGameState()
{
	if (!HasAuthority())
	{
		return;
	}

	AWjWorldGameStateWaitingRoom* WaitingRoomGameState = GetGameState<AWjWorldGameStateWaitingRoom>();
	if (!WaitingRoomGameState)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("WjWorldGameModeWaitingRoom: GameState not found"));
		return;
	}

	// 호스트의 로컬 SaveGame 로드
	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("LobbyLayout"), 0)
	);

	if (SaveGame && SaveGame->PlacedObjects.Num() > 0)
	{
		WaitingRoomGameState->SetPlacedObjects(SaveGame->PlacedObjects);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeWaitingRoom: Host layout loaded to GameState (%d objects)"),
			SaveGame->PlacedObjects.Num());
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeWaitingRoom: No saved layout found for host"));
	}
}
