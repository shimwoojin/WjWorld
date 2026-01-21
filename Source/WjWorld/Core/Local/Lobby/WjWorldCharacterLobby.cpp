// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldCharacterLobby.h"

AWjWorldCharacterLobby::AWjWorldCharacterLobby()
{
}

void AWjWorldCharacterLobby::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeLobbyCharacter();
}

void AWjWorldCharacterLobby::InitializeCharacter()
{
	Super::InitializeCharacter();
	
	// 로비 캐릭터 초기화
}

void AWjWorldCharacterLobby::SetupInputBindings(UInputComponent* PlayerInputComponent)
{
	Super::SetupInputBindings(PlayerInputComponent);
	
	// 로비 전용 입력 바인딩
}

void AWjWorldCharacterLobby::UpdateCharacterAppearance()
{
	// 캐릭터 외형 업데이트 로직
}

void AWjWorldCharacterLobby::InitializeLobbyCharacter()
{
}
