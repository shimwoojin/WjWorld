// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldCharacterWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticComponent.h"

AWjWorldCharacterWaitingRoom::AWjWorldCharacterWaitingRoom()
{
}

void AWjWorldCharacterWaitingRoom::BeginPlay()
{
	Super::BeginPlay();

	InitializeWaitingRoomCharacter();
}

void AWjWorldCharacterWaitingRoom::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// PlayerState에 Pawn 설정 알림 → 대기 중인 코스메틱 적용
	if (AWjWorldPlayerStateBase* PS = GetPlayerState<AWjWorldPlayerStateBase>())
	{
		PS->OnPawnSet(nullptr, this);
	}
}

void AWjWorldCharacterWaitingRoom::InitializeCharacter()
{
	Super::InitializeCharacter();
	
	// 대기실 캐릭터 초기화
}

void AWjWorldCharacterWaitingRoom::SetupInputBindings(UInputComponent* PlayerInputComponent)
{
	Super::SetupInputBindings(PlayerInputComponent);
	
	// 대기실 전용 입력 바인딩
}

void AWjWorldCharacterWaitingRoom::UpdateReadyState(bool bNewIsReady)
{
	bIsReady = bNewIsReady;
	
	// 준비 상태에 따른 외형 변경 로직
	// 예: 준비 완료 시 특정 애니메이션 재생, 이펙트 표시 등
}

void AWjWorldCharacterWaitingRoom::UpdateTeamAppearance(int32 TeamID)
{
	CurrentTeamID = TeamID;
	
	// 팀에 따른 외형 변경 로직
	// 예: 팀 컬러 적용, 팀 엠블럼 표시 등
}

void AWjWorldCharacterWaitingRoom::InitializeWaitingRoomCharacter()
{
	// 대기실 전용 초기화 로직
	bIsReady = false;
	CurrentTeamID = 0;
}
