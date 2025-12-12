// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/Local/WaitingRoom/WjWorldHUDWaitingRoom.h"
#include "Core/Local/WaitingRoom/WjWorldGameStateWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
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

	// ⭐ GameState 초기화 (Server Only)
	if (HasAuthority())
	{
		UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
		if (GameInstance && GameInstance->GetSessionManager())
		{
			AWjWorldGameStateWaitingRoom* WjGameState = GetGameState<AWjWorldGameStateWaitingRoom>();
			if (WjGameState)
			{
				// SessionManager에서 방 설정 가져오기
				const FRoomSettings& Settings = GameInstance->GetSessionManager()->GetLastRoomSettings();
				WjGameState->InitializeRoomSettings(Settings);

				UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: GameState initialized with room settings"));
			}
			else
			{
				UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: Failed to get GameStateWaitingRoom"));
			}
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: GameInstance or SessionManager is null"));
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

	// GameInstance를 통해 세션 시작
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		bool bSuccess = GameInstance->StartGame();
		if (bSuccess)
		{
			UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Session start initiated"));
			
			// TODO: 게임 맵으로 이동
			// GetWorld()->ServerTravel("/Game/Maps/GameMap");
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: Failed to start session"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeWaitingRoom: GameInstance is null"));
	}
}
