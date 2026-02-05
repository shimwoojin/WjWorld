// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Sumo/SumoFloorRingActor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "WjWorldLogCategories.h"

ASumoFloorRingActor::ASumoFloorRingActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	RootComponent = FloorMesh;
	FloorMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void ASumoFloorRingActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASumoFloorRingActor, RingState);
}

void ASumoFloorRingActor::StartWarning()
{
	if (RingState != ESumoRingState::Active) return;
	if (RingOrder == 0) return; // 중앙 링은 파괴 불가

	RingState = ESumoRingState::Warning;
	ApplyRingStateVisual();

	UE_LOG(LogWjWorld, Log, TEXT("SumoFloorRing[%d]: Warning started"), RingOrder);
}

void ASumoFloorRingActor::DestroyRing()
{
	if (RingState == ESumoRingState::Destroyed) return;
	if (RingOrder == 0) return;

	RingState = ESumoRingState::Destroyed;
	ApplyRingStateVisual();

	UE_LOG(LogWjWorld, Log, TEXT("SumoFloorRing[%d]: Destroyed"), RingOrder);
}

void ASumoFloorRingActor::ResetRing()
{
	RingState = ESumoRingState::Active;
	ApplyRingStateVisual();

	UE_LOG(LogWjWorld, Log, TEXT("SumoFloorRing[%d]: Reset"), RingOrder);
}

void ASumoFloorRingActor::OnRep_RingState()
{
	ApplyRingStateVisual();
}

void ASumoFloorRingActor::ApplyRingStateVisual()
{
	if (!FloorMesh) return;

	// 기본 머티리얼 캐시
	if (!DefaultMaterial && FloorMesh->GetNumMaterials() > 0)
	{
		DefaultMaterial = FloorMesh->GetMaterial(0);
	}

	switch (RingState)
	{
	case ESumoRingState::Active:
		FloorMesh->SetVisibility(true);
		FloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		if (DefaultMaterial)
		{
			FloorMesh->SetMaterial(0, DefaultMaterial);
		}
		break;

	case ESumoRingState::Warning:
		FloorMesh->SetVisibility(true);
		FloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		if (WarningMaterial)
		{
			FloorMesh->SetMaterial(0, WarningMaterial);
		}
		break;

	case ESumoRingState::Destroyed:
		FloorMesh->SetVisibility(false);
		FloorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}
