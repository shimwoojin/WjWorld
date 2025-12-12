// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/Base/WjWorldGameModeBase.h"
#include "Core/Base/WjWorldGameStateBase.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/Base/WjWorldHUDBase.h"

AWjWorldGameModeBase::AWjWorldGameModeBase()
{
	// ⭐ 중요: PlayerState 클래스 설정
	PlayerStateClass = AWjWorldPlayerStateBase::StaticClass();
	
	// 기본 GameState 클래스 설정
	GameStateClass = AWjWorldGameStateBase::StaticClass();
	
	// 기본 HUD 클래스 설정
	HUDClass = AWjWorldHUDBase::StaticClass();
}
