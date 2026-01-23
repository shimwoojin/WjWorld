// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldTileActor.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/WjWorldGameplayUtils.h"
#include "Components/BoxComponent.h"

#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "WjWorldLogCategories.h"

AWjWorldTileActor::AWjWorldTileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CenterHitBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CenterHitBoxComponent"));
	CenterHitBoxComponent->SetupAttachment(RootComponent);
	CenterHitBoxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CenterHitBoxComponent->SetBoxExtent(FVector(HitBoxSize, HitBoxSize, HitBoxSize));

	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		HitBoxComponents[DirectionIndex] = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("HitBoxComponent_%d"), DirectionIndex));
		HitBoxComponents[DirectionIndex]->SetupAttachment(RootComponent);
		HitBoxComponents[DirectionIndex]->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		HitBoxComponents[DirectionIndex]->SetBoxExtent(FVector(HitBoxSize, HitBoxSize, HitBoxSize));
	}
}

void AWjWorldTileActor::InitializeTile(const FVector& InSize, const FVector& InCenterOffset)
{
	SetActorLocation(InCenterOffset);

	float Offset = HitBoxSize + 2.0f;

	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		if (HitBoxComponents[DirectionIndex] == nullptr) continue;

		FVector BoxLocation = FVector::ZeroVector;

		switch (DirectionIndex)
		{
		case EWjWorldDirection::Up:
		{
			BoxLocation = FVector(0.0f, InSize.Y, 0.0f);
			break;
		}

		case EWjWorldDirection::Right:
		{
			BoxLocation = FVector(InSize.X, 0.0f, 0.0f);
			break;
		}

		case EWjWorldDirection::Down:
		{
			BoxLocation = FVector(0.0f, -InSize.Y, 0.0f);
			break;
		}

		case EWjWorldDirection::Left:
		{
			BoxLocation = FVector(-InSize.X, 0, 0.0f);
			break;
		}
		}

		HitBoxComponents[DirectionIndex]->SetRelativeLocation(BoxLocation);
		HitBoxComponents[DirectionIndex]->OnComponentBeginOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapBegin);
		HitBoxComponents[DirectionIndex]->OnComponentEndOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapEnd);
	}

	CenterHitBoxComponent->SetBoxExtent(InSize);
	CenterHitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapBegin);
	CenterHitBoxComponent->OnComponentEndOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapEnd);
}

// Called when the game starts or when spawned
void AWjWorldTileActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AWjWorldGameModePlay* GameModePlay = GetWorld()->GetAuthGameMode<AWjWorldGameModePlay>();
		if (GameModePlay)
		{
			GameRule = GameModePlay->GetCurrentGameRule<UWjWorldGameRuleApproachingWall>();
		}
	}
}

// Called every frame
void AWjWorldTileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;
	if (bIsBombSignalOn == false) return;

	ElapsedBombingTime += DeltaTime;

	if (ElapsedBombingTime >= BombChargingTime)
	{
		Bomb();
		ElapsedBombingTime = 0.0f;
		bIsBombSignalOn = false;
	}
}

void AWjWorldTileActor::OnBrickOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 처리
	if (GetLocalRole() != ROLE_Authority) return;
	
	AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(OtherActor);
	if (!BrickActor) return;

	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		if (HitBoxComponents[DirectionIndex] == OverlappedComponent)
		{
			++bIsOverlapBricks[DirectionIndex];
		}
	}

	if (WjWorldGameplayUtils::IsGameplayPhaseplaying(GetWorld()))
	{
		if (bIsBombSignalOn == false && CheckBombSignalOn())
		{
			bIsBombSignalOn = true;
			ElapsedBombingTime = 0.0f;
		}
	}
}

void AWjWorldTileActor::OnBrickOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetLocalRole() != ROLE_Authority) return;

	AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(OtherActor);
	if (!BrickActor) return;

	if (OverlappedComponent == CenterHitBoxComponent)
	{
		const FWjWorldBrickProperties& BrickProperties = BrickActor->GetBrickComponent()->GetBrickProperties();
		if (BrickProperties.BrickType == EWjWorldBrickType::Standard)
		{
			Destroy();
			return;
		}
	}
	else
	{
		for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
		{
			if (HitBoxComponents[DirectionIndex] == OverlappedComponent)
			{
				--bIsOverlapBricks[DirectionIndex];
				bIsOverlapBricks[DirectionIndex] = FMath::Max(0, bIsOverlapBricks[DirectionIndex]);
			}
		}
	}

	if (WjWorldGameplayUtils::IsGameplayPhaseplaying(GetWorld()))
	{
		if (bIsBombSignalOn && !CheckBombSignalOn())
		{
			bIsBombSignalOn = false;
			ElapsedBombingTime = 0.0f;
		}
	}
}

bool AWjWorldTileActor::CheckBombSignalOn() const
{
	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		if (bIsOverlapBricks[DirectionIndex] <= 0)
		{
			return false;
		}
	}

	return true;
}

void AWjWorldTileActor::Bomb()
{
	if (GetLocalRole() != ROLE_Authority) return;
	if (WjWorldGameplayUtils::IsGameplayPhaseplaying(GetWorld()) == false) return;

	TArray<AActor*> BombedActors;
	CenterHitBoxComponent->GetOverlappingActors(BombedActors, AWjWorldCharacterPlay::StaticClass());

	for (AActor* BombedActor : BombedActors)
	{
		AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(BombedActor);
		if (Character == nullptr) continue;

		if (GameRule.IsValid())
		{
			GameRule->OnPlayerEliminated(Character);
		}
	}

	SpawnBombEffect();
}

void AWjWorldTileActor::SpawnBombEffect_Implementation()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		NiagaraSystem,  // UNiagaraSystem*
		GetActorLocation(),
		GetActorRotation(),
		GetActorScale3D(),  // Scale
		true,           // Auto Destroy
		true,           // Auto Activate
		ENCPoolMethod::None
	);

	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldTileActor::SpawnBombEffect_Implementation"));
}
