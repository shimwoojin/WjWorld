// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldWallManager.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"
#include "Components/BoxComponent.h"

#include "Engine/OverlapResult.h"

#include "Net/UnrealNetwork.h"

const TCHAR* UWjWorldBrickComponent::BrickMeshPath = TEXT("/Game/GamePlay/Wall/Mesh/Cube");

UWjWorldBrickComponent::UWjWorldBrickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWjWorldBrickComponent::InitializeBrick(const FWjWorldBrickProperties& InBrickProperties)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		this->BrickProperties = InBrickProperties;

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

	if (BrickMeshComponent)
	{
		// Set mesh, size, and color based on BrickProperties
		// This is a placeholder; actual implementation would depend on available assets and materials
		FVector Scale = BrickProperties.Size / 100.0f; // Assuming the default mesh size is 100 units
		BrickMeshComponent->SetWorldScale3D(Scale);

		UStaticMesh* BrickMesh = LoadObject<UStaticMesh>(this, BrickMeshPath);
		if (BrickMesh)
		{
			BrickMeshComponent->SetStaticMesh(BrickMesh);
		}

		UMaterialInstanceDynamic* DynamicMaterial = BrickMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor(BrickProperties.Color));
		}
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
	//BrickMeshComponent->SetIsReplicated(true);
}

void UWjWorldBrickComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UWjWorldBrickComponent, BrickProperties, COND_InitialOnly);
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

		if (BrickProperties.BrickType == EWjWorldBrickType::Standard)
		{
			BrickMovement->MoveBrick(UWjWorldWallManager::GetBrickMovementAllowedTime(BrickMoveSignalCount));
		}
	}
}

void UWjWorldBrickComponent::HandleWallCollision(const FVector& WallDirection)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	OnWallCollision.Broadcast(this, WallDirection);

	switch (BrickProperties.BrickType)
	{
	case EWjWorldBrickType::Explosive:
		Explode();
		DestroyBrick();
		break;

	case EWjWorldBrickType::Destructible:
		DestroyBrick();
		break;

	case EWjWorldBrickType::Moving:
		PushInDirection(WallDirection, UWjWorldWallManager::GetBrickMovementAllowedTime(CurrentBrickMoveSignalCount));
		break;

	case EWjWorldBrickType::Standard:
		// Standard는 벽 자체이므로 충돌 처리 없음
		break;
	}
}

void UWjWorldBrickComponent::PushInDirection(const FVector& Direction, float MoveTime)
{
	if (!BrickMovement) return;

	BrickMovement->PushBrick(Direction, MoveTime);
}

void UWjWorldBrickComponent::Explode()
{
	if (GetOwnerRole() != ROLE_Authority) return;

	// 상하좌우 데미지 처리
	const FVector& BrickSize = BrickProperties.Size;
	const FVector CurrentLocation = GetComponentLocation();

	TArray<FVector> ExplosionDirections = {
		FVector(BrickSize.X, 0.0f, 0.0f),   // Right
		FVector(-BrickSize.X, 0.0f, 0.0f),  // Left
		FVector(0.0f, BrickSize.Y, 0.0f),   // Up
		FVector(0.0f, -BrickSize.Y, 0.0f)   // Down
	};

	for (const FVector& Dir : ExplosionDirections)
	{
		FVector CheckLocation = CurrentLocation + Dir;

		// 해당 위치의 캐릭터나 다른 오브젝트에 데미지
		TArray<FOverlapResult> Overlaps;
		FCollisionShape CollisionShape = FCollisionShape::MakeBox(BrickSize * 0.4f);

		if (GetWorld()->OverlapMultiByChannel(
			Overlaps,
			CheckLocation,
			FQuat::Identity,
			ECC_Pawn,
			CollisionShape))
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				if (AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(Overlap.GetActor()))
				{
					// 캐릭터에 데미지 또는 제거 처리
					if (UWjWorldGameRuleApproachingWall* GameRule = Cast<UWjWorldGameRuleApproachingWall>(GetGameModePlay()->GetCurrentGameRule()))
					{
						GameRule->OnPlayerEliminated(Character);
					}
				}
			}
		}
	}
}

void UWjWorldBrickComponent::DestroyBrick()
{
	ReserveDestroyBrick(0.1f);
}
