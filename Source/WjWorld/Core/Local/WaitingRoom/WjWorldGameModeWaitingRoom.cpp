// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/WaitingRoom/WjWorldGameModeWaitingRoom.h"
#include "Core/Local/WaitingRoom/WjWorldHUDWaitingRoom.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeWaitingRoom::AWjWorldGameModeWaitingRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	// 기본 HUD 클래스 설정
	HUDClass = AWjWorldHUDWaitingRoom::StaticClass();

	// 초기 플레이어 수
	PlayerCount = 0;
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

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: BeginPlay - WaitingRoom loaded"));
}

void AWjWorldGameModeWaitingRoom::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerCount++;
	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Player joined - Total: %d"), PlayerCount);

	// TODO: 모든 클라이언트에게 플레이어 목록 업데이트 브로드캐스트
}

void AWjWorldGameModeWaitingRoom::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	PlayerCount--;
	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeWaitingRoom: Player left - Total: %d"), PlayerCount);

	// TODO: 모든 클라이언트에게 플레이어 목록 업데이트 브로드캐스트
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
