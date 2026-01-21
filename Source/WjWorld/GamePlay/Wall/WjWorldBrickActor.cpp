// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"

// Sets default values
AWjWorldBrickActor::AWjWorldBrickActor()
{
	PrimaryActorTick.bCanEverTick = true;

	BrickComponent = CreateDefaultSubobject<UWjWorldBrickComponent>(TEXT("BrickComponent"));
	//BrickComponent->SetupAttachment(RootComponent);
	SetRootComponent(BrickComponent);
	BrickComponent->SetIsReplicated(true);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AWjWorldBrickActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWjWorldBrickActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

