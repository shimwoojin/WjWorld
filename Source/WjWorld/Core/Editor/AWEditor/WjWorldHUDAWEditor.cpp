// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Editor/AWEditor/WjWorldHUDAWEditor.h"
#include "Core/Editor/AWEditor/WjWorldPlayerControllerAWEditor.h"
#include "UI/Placement/PlacementHUDWidgetAWEditor.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "WjWorldLogCategories.h"

AWjWorldHUDAWEditor::AWjWorldHUDAWEditor()
{
}

void AWjWorldHUDAWEditor::BeginPlay()
{
	Super::BeginPlay();

	// PlayerController의 BeginPlay가 먼저 실행되도록 다음 틱에서 HUD 위젯 생성
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AWjWorldHUDAWEditor::CreateHUDWidget);
}

void AWjWorldHUDAWEditor::CreateHUDWidget()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("HUDAWEditor: No HUDWidgetClass set"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	HUDWidget = CreateWidget<UPlacementHUDWidgetAWEditor>(PC, HUDWidgetClass);
	if (HUDWidget)
	{
		// PlacementComponent 연결
		AWjWorldPlayerControllerAWEditor* EditorPC = Cast<AWjWorldPlayerControllerAWEditor>(PC);
		if (EditorPC)
		{
			UWjWorldPlacementComponent* PlacementComp = EditorPC->GetPlacementComponent();
			if (PlacementComp)
			{
				HUDWidget->SetPlacementComponent(PlacementComp);
			}
		}

		HUDWidget->AddToViewport();
		UE_LOG(LogWjWorldPlacement, Log, TEXT("HUDAWEditor: HUD widget created and PlacementComponent connected"));
	}
}
