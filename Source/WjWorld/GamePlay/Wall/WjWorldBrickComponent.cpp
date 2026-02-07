// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldWallManager.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Components/BoxComponent.h"

#include "Engine/OverlapResult.h"

#include "Net/UnrealNetwork.h"

UStaticMesh* UWjWorldBrickComponent::GetBrickMesh()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings && !Settings->BrickMesh.IsNull())
	{
		return Settings->BrickMesh.LoadSynchronous();
	}
	return nullptr;
}

UWjWorldBrickComponent::UWjWorldBrickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UWjWorldBrickComponent::InitializeBrick(const FWjWorldBrickProperties& InBrickProperties)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		this->BrickProperties = InBrickProperties;
		this->CurrentHP = InBrickProperties.MaxHP;

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
		FVector Scale = BrickProperties.Size / 100.0f; // Assuming the default mesh size is 100 units
		BrickMeshComponent->SetWorldScale3D(Scale);

		// 대각선 이동을 위해 collision을 약간 축소 (시각적 크기 유지, collision만 95%)
		// StaticMesh collision 비활성화 후 별도 BoxComponent로 처리
		BrickMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		UStaticMesh* BrickMesh = GetBrickMesh();
		if (BrickMesh)
		{
			BrickMeshComponent->SetStaticMesh(BrickMesh);
		}

		UMaterialInstanceDynamic* DynamicMaterial = BrickMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor(BrickProperties.GetColorWithBrickType()));
			DynamicMaterial->SetScalarParameterValue(FName("CrackIntensity"), 0.0f);
		}
	}

	// BlockingCollisionComponent 크기 설정 (실제 크기의 95%로 대각선 통과 가능하게)
	if (BlockingCollisionComponent)
	{
		// BoxExtent는 half-size이므로 Size의 절반을 사용
		const FVector CollisionHalfSize = BrickProperties.Size * 0.475f; // 95% / 2 = 0.475
		BlockingCollisionComponent->SetBoxExtent(CollisionHalfSize);
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

	CenterHitBoxComponent = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("CenterHitBoxComponent"));
	CenterHitBoxComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	CenterHitBoxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CenterHitBoxComponent->SetBoxExtent(FVector(HitBoxSize, HitBoxSize, HitBoxSize));

	// 캐릭터 blocking collision용 BoxComponent (대각선 이동을 위해 크기 축소)
	BlockingCollisionComponent = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("BlockingCollisionComponent"));
	BlockingCollisionComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	BlockingCollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	BlockingCollisionComponent->SetBoxExtent(FVector(HitBoxSize, HitBoxSize, HitBoxSize)); // BeginPlay에서 실제 크기 설정

	BrickMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("BrickMeshComponent"));
	BrickMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	BrickMeshComponent->RegisterComponent();
	//BrickMeshComponent->SetIsReplicated(true);
}

void UWjWorldBrickComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UWjWorldBrickComponent, BrickProperties, COND_InitialOnly);
	DOREPLIFETIME(UWjWorldBrickComponent, CurrentHP);
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
		// 벽 충돌은 HP 무시하고 즉시 파괴 + 파쇄 연출
		if (AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(GetOwner()))
		{
			BrickActor->MulticastSpawnDestructionFracture();
		}
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

	// 폭발 VFX
	if (AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(GetOwner()))
	{
		BrickActor->MulticastSpawnExplosionEffect();
	}

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
	// 파괴 VFX
	if (AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(GetOwner()))
	{
		BrickActor->MulticastSpawnDestroyEffect();
	}

	ReserveDestroyBrick(0.1f);
}

void UWjWorldBrickComponent::ApplyDamage(int32 DamageAmount)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (CurrentHP <= 0) return;

	CurrentHP = FMath::Max(0, CurrentHP - DamageAmount);

	if (CurrentHP <= 0)
	{
		// HP 소진 → 파쇄 연출 + 파괴
		if (AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(GetOwner()))
		{
			BrickActor->MulticastSpawnDestructionFracture();
		}
		DestroyBrick();
	}
	else
	{
		// 서버에서도 비주얼 업데이트 (클라이언트는 OnRep에서 호출)
		UpdateDamageVisuals();
	}
}

void UWjWorldBrickComponent::OnRep_CurrentHP()
{
	UpdateDamageVisuals();
}

void UWjWorldBrickComponent::UpdateDamageVisuals()
{
	if (!BrickMeshComponent) return;

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(BrickMeshComponent->GetMaterial(0));
	if (!DynamicMaterial) return;

	const int32 MaxHP = BrickProperties.MaxHP;
	if (MaxHP <= 0) return;

	// HP 비율: 1.0(풀HP) → 0.0(체력 없음)
	const float HPRatio = static_cast<float>(CurrentHP) / static_cast<float>(MaxHP);

	// BaseColor: Black(풀HP) → DarkRed(중간) → Red(저HP)
	const FLinearColor FullHPColor = FLinearColor::Black;
	const FLinearColor LowHPColor = FLinearColor::Red;
	const FLinearColor InterpolatedColor = FMath::Lerp(LowHPColor, FullHPColor, HPRatio);
	DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), InterpolatedColor);

	// CrackIntensity: 0.0(풀HP) → 1.0(저HP)
	const float CrackIntensity = 1.0f - HPRatio;
	DynamicMaterial->SetScalarParameterValue(FName("CrackIntensity"), CrackIntensity);
}
