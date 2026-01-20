// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameStatePlay.h"
#include "Core/Play/WjWorldHUDPlay.h"
#include "Core/Play/WjWorldPlayerControllerPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"

AWjWorldGameModePlay::AWjWorldGameModePlay()
{
	PlayerStateClass = AWjWorldPlayerStatePlay::StaticClass();
	PlayerControllerClass = AWjWorldPlayerControllerPlay::StaticClass();
	HUDClass = AWjWorldHUDPlay::StaticClass();
	GameStateClass = AWjWorldGameStatePlay::StaticClass();
	DefaultPawnClass = AWjWorldCharacterPlay::StaticClass();
}
