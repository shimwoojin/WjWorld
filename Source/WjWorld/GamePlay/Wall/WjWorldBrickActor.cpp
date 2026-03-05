// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"

#include "Setting/WjWorldDeveloperSettings.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"

#include "WjWorldLogCategories.h"

// Sets default values
AWjWorldBrickActor::AWjWorldBrickActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BrickComponent = CreateDefaultSubobject<UWjWorldBrickComponent>(TEXT("BrickComponent"));
	SetRootComponent(BrickComponent);
	BrickComponent->SetIsReplicated(true);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AWjWorldBrickActor::BeginPlay()
{
	Super::BeginPlay();
}

void AWjWorldBrickActor::PostNetInit()
{
	Super::PostNetInit();
}

void AWjWorldBrickActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWjWorldBrickActor::MulticastSpawnDestroyEffect_Implementation()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	UNiagaraSystem* Effect = Settings->BrickDestroyEffect.LoadSynchronous();
	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Effect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None
		);
	}
}

void AWjWorldBrickActor::MulticastSpawnExplosionEffect_Implementation()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	UNiagaraSystem* Effect = Settings->BrickExplosionEffect.LoadSynchronous();
	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Effect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None
		);
	}
}

void AWjWorldBrickActor::MulticastSpawnDamageHitEffect_Implementation()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings->BrickDamageHitEffect.IsNull()) return;

	UNiagaraSystem* Effect = Settings->BrickDamageHitEffect.LoadSynchronous();
	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Effect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None
		);
	}
}

void AWjWorldBrickActor::MulticastSpawnDestructionFracture_Implementation()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	UGeometryCollection* FractureCollection = Settings->DestructibleBrickFractureCollection.LoadSynchronous();
	if (!FractureCollection) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector BrickLocation = GetActorLocation();

	// 벽돌 크기 기반 스케일 계산 (기본 메시 100단위 가정)
	FVector BrickScale = FVector::OneVector;
	if (BrickComponent)
	{
		const FVector& BrickSize = BrickComponent->GetBrickProperties().Size;
		BrickScale = BrickSize / 100.0f;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGeometryCollectionActor* FractureActor = World->SpawnActor<AGeometryCollectionActor>(
		AGeometryCollectionActor::StaticClass(),
		BrickLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (FractureActor)
	{
		FractureActor->SetActorScale3D(BrickScale);

		UGeometryCollectionComponent* GCComponent = FractureActor->GetGeometryCollectionComponent();
		if (GCComponent)
		{
			GCComponent->SetRestCollection(FractureCollection);
			GCComponent->SetSimulatePhysics(true);
		}

		FractureActor->SetLifeSpan(Settings->DestructibleBrickFractureLifetime);
	}
}
