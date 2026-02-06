// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/JumpMapEditor/WjWorldGameModeJumpMapEditor.h"
#include "Core/Editor/JumpMapEditor/WjWorldGameStateJumpMapEditor.h"
#include "Core/Editor/JumpMapEditor/WjWorldPlayerControllerJumpMapEditor.h"
#include "Core/Editor/JumpMapEditor/WjWorldHUDJumpMapEditor.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeJumpMapEditor::AWjWorldGameModeJumpMapEditor()
{
	// 기본 클래스 설정
	GameStateClass = AWjWorldGameStateJumpMapEditor::StaticClass();
	PlayerControllerClass = AWjWorldPlayerControllerJumpMapEditor::StaticClass();
	HUDClass = AWjWorldHUDJumpMapEditor::StaticClass();
	DefaultPawnClass = AWjWorldCharacterBase::StaticClass();

	// 싱글플레이어 에디터 모드
	bUseSeamlessTravel = false;
}

void AWjWorldGameModeJumpMapEditor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogWjWorldPlacement, Log, TEXT("GameModeJumpMapEditor: BeginPlay - JumpMap Editor started"));
}
