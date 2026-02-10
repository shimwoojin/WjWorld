// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapActorBase.h"
#include "Components/StaticMeshComponent.h"

AJumpMapActorBase::AJumpMapActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComp);
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AJumpMapActorBase::InitializeFromLayoutEntry(const FTransform& InTransform)
{
	SetActorTransform(InTransform);
}
