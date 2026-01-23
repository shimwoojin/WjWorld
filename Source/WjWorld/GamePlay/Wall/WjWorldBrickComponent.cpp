// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldWallManager.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"
#include "Components/BoxComponent.h"

const TCHAR* UWjWorldBrickComponent::BrickMeshPath = TEXT("/Game/GamePlay/Wall/Mesh/Cube");

UWjWorldBrickComponent::UWjWorldBrickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWjWorldBrickComponent::InitializeBrick(const FWjWorldBrickProperties& InBrickProperties)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	this->BrickProperties = InBrickProperties;

	if (BrickMeshComponent)
	{
		// Set mesh, size, and color based on BrickProperties
		// This is a placeholder; actual implementation would depend on available assets and materials
		FVector Scale = InBrickProperties.Size / 100.0f; // Assuming the default mesh size is 100 units
		BrickMeshComponent->SetWorldScale3D(Scale);

		UStaticMesh* BrickMesh = LoadObject<UStaticMesh>(this, BrickMeshPath);
		if(BrickMesh)
		{
			BrickMeshComponent->SetStaticMesh(BrickMesh);

			UMaterialInstanceDynamic* DynamicMaterial = BrickMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor(InBrickProperties.Color));
			}
		}
	}

	switch (InBrickProperties.BrickMoveType)
	{
		case EWjWorldBrickMoveType::Standard:
		{
			BrickMovement = NewObject<UWjWorldBrickMovement>(this, UWjWorldBrickMovement::StaticClass());
			BrickMovement->Initialize(this, Cast<UWjWorldGameRuleApproachingWall>(GetGameModePlay()->GetCurrentGameRule()));
			break;
		}
	}
}

void UWjWorldBrickComponent::ReserveDestroyBrick(float AfterSeconds)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this]() {
			if (AActor* OwnerActor = GetOwner())
			{
				OwnerActor->Destroy();
			}
			}, AfterSeconds, false);
	}
}

// Called when the game starts
void UWjWorldBrickComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetGameModePlay()->OnGameLevelChange.AddUObject(this, &UWjWorldBrickComponent::OnBrickMovementSignal);
	}
}

void UWjWorldBrickComponent::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetGameModePlay()->OnGameLevelChange.RemoveAll(this);
		GetWorld()->GetTimerManager().ClearTimer(DestroyHandle);
	}
}

void UWjWorldBrickComponent::OnRegister()
{
	Super::OnRegister();

	BrickMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("BrickMeshComponent"));
	BrickMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	BrickMeshComponent->RegisterComponent();
	BrickMeshComponent->SetIsReplicated(true);
}


// Called every frame
void UWjWorldBrickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_Authority)
	{
		if (BrickMovement)
		{
			BrickMovement->Tick(DeltaTime);
		}
	}
}

void UWjWorldBrickComponent::OnBrickMovementSignal(int32 BrickMoveSignalCount)
{
	if (BrickMovement)
	{
		CurrentBrickMoveSignalCount = BrickMoveSignalCount;
		BrickMovement->MoveBrick(UWjWorldWallManager::GetBrickMovementAllowedTime(BrickMoveSignalCount));
	}
}
