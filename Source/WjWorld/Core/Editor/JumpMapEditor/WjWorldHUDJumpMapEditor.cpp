// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/JumpMapEditor/WjWorldHUDJumpMapEditor.h"
#include "Core/Editor/JumpMapEditor/WjWorldPlayerControllerJumpMapEditor.h"
#include "UI/Placement/PlacementHUDWidgetJumpMapEditor.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "WjWorldLogCategories.h"

AWjWorldHUDJumpMapEditor::AWjWorldHUDJumpMapEditor()
{
}

void AWjWorldHUDJumpMapEditor::BeginPlay()
{
	Super::BeginPlay();

	// PlayerController의 BeginPlay가 먼저 실행되도록 다음 틱에서 HUD 위젯 생성
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AWjWorldHUDJumpMapEditor::CreateHUDWidget);
}

void AWjWorldHUDJumpMapEditor::CreateHUDWidget()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("HUDJumpMapEditor: No HUDWidgetClass set"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	HUDWidget = CreateWidget<UPlacementHUDWidgetJumpMapEditor>(PC, HUDWidgetClass);
	if (HUDWidget)
	{
		// PlacementComponent 연결
		AWjWorldPlayerControllerJumpMapEditor* EditorPC = Cast<AWjWorldPlayerControllerJumpMapEditor>(PC);
		if (EditorPC)
		{
			UWjWorldPlacementComponent* PlacementComp = EditorPC->GetPlacementComponent();
			if (PlacementComp)
			{
				HUDWidget->SetPlacementComponent(PlacementComp);
			}
		}

		HUDWidget->AddToViewport();
		UE_LOG(LogWjWorldPlacement, Log, TEXT("HUDJumpMapEditor: HUD widget created and PlacementComponent connected"));
	}
}
