// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlayerControllerWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/Local/WaitingRoom/WjWorldGameStateWaitingRoom.h"

AWjWorldPlayerControllerWaitingRoom::AWjWorldPlayerControllerWaitingRoom()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AWjWorldPlayerControllerWaitingRoom::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeWaitingRoomController();
}

void AWjWorldPlayerControllerWaitingRoom::InitializeController()
{
	Super::InitializeController();
	
	// 대기실 컨트롤러 초기화
	SetInputMode(FInputModeGameAndUI());
}

void AWjWorldPlayerControllerWaitingRoom::InitializeUI()
{
	Super::InitializeUI();
	
	CreateWaitingRoomUI();
}

void AWjWorldPlayerControllerWaitingRoom::ToggleReady()
{
	ServerToggleReady();
}

void AWjWorldPlayerControllerWaitingRoom::RequestChangeTeam(int32 NewTeamID)
{
	ServerRequestChangeTeam(NewTeamID);
}

bool AWjWorldPlayerControllerWaitingRoom::CanStartGame() const
{
	// GameState를 통해 모든 플레이어가 준비되었는지 확인
	if (AWjWorldGameStateWaitingRoom* WaitingRoomGS = GetWorld()->GetGameState<AWjWorldGameStateWaitingRoom>())
	{
		return WaitingRoomGS->AreAllPlayersReady();
	}
	
	return false;
}

void AWjWorldPlayerControllerWaitingRoom::InitializeWaitingRoomController()
{
	// 대기실 컨트롤러 전용 초기화 로직
}

void AWjWorldPlayerControllerWaitingRoom::CreateWaitingRoomUI()
{
	// 대기실 UI 생성 로직
}

void AWjWorldPlayerControllerWaitingRoom::ServerToggleReady_Implementation()
{
	if (AWjWorldPlayerStateBase* PS = GetPlayerState<AWjWorldPlayerStateBase>())
	{
		PS->ToggleReady();
	}
}

void AWjWorldPlayerControllerWaitingRoom::ServerRequestChangeTeam_Implementation(int32 NewTeamID)
{
	// 팀 변경 요청 처리 로직
	// GameMode를 통해 팀 배정 관리
}
