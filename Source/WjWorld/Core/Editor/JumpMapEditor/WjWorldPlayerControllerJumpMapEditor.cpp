// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/JumpMapEditor/WjWorldPlayerControllerJumpMapEditor.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "WjWorldLogCategories.h"

AWjWorldPlayerControllerJumpMapEditor::AWjWorldPlayerControllerJumpMapEditor()
{
	// PlacementComponent 생성
	PlacementComponent = CreateDefaultSubobject<UWjWorldPlacementComponent>(TEXT("PlacementComponent"));
}

void AWjWorldPlayerControllerJumpMapEditor::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어인 경우에만 배치 모드 자동 진입
	if (IsLocalController())
	{
		// InputMode를 GameAndUI로 설정 (UI 클릭 + 게임 입력 동시 사용)
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);

		// JumpMap 컨텍스트로 배치 모드 진입
		if (PlacementComponent)
		{
			PlacementComponent->EnterPlacementModeWithContext(EPlacementContext::JumpMap);
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlayerControllerJumpMapEditor: Entered placement mode (JumpMap context)"));
		}
	}
}
