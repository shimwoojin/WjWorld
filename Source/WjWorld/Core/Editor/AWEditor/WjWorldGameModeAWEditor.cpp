// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/AWEditor/WjWorldGameModeAWEditor.h"
#include "Core/Editor/AWEditor/WjWorldGameStateAWEditor.h"
#include "Core/Editor/AWEditor/WjWorldPlayerControllerAWEditor.h"
#include "Core/Editor/AWEditor/WjWorldHUDAWEditor.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeAWEditor::AWjWorldGameModeAWEditor()
{
	// 기본 클래스 설정
	GameStateClass = AWjWorldGameStateAWEditor::StaticClass();
	PlayerControllerClass = AWjWorldPlayerControllerAWEditor::StaticClass();
	HUDClass = AWjWorldHUDAWEditor::StaticClass();
	DefaultPawnClass = AWjWorldCharacterBase::StaticClass();

	// 싱글플레이어 에디터 모드
	bUseSeamlessTravel = false;
}

void AWjWorldGameModeAWEditor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameModeAWEditor: BeginPlay - Approaching Wall Editor started"));
}
