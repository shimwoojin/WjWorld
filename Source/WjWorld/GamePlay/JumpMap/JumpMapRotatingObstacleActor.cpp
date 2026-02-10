// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapRotatingObstacleActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleJumpMap.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WjWorldLogCategories.h"

AJumpMapRotatingObstacleActor::AJumpMapRotatingObstacleActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("HitTrigger"));
	HitTrigger->SetupAttachment(RootComp);
	HitTrigger->SetBoxExtent(FVector(200.f, 50.f, 50.f));
	HitTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HitTrigger->SetGenerateOverlapEvents(true);
}

void AJumpMapRotatingObstacleActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		HitTrigger->OnComponentBeginOverlap.AddDynamic(this, &AJumpMapRotatingObstacleActor::OnHitOverlap);
	}
}

void AJumpMapRotatingObstacleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FRotator DeltaRotation = RotationAxis * RotationSpeed * DeltaTime;
	AddActorLocalRotation(DeltaRotation);
}

void AJumpMapRotatingObstacleActor::OnHitOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(OtherActor);
	if (!Character || Character->IsEliminated()) return;

	if (bKillOnHit)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			AWjWorldGameModePlay* GameMode = Cast<AWjWorldGameModePlay>(World->GetAuthGameMode());
			if (GameMode)
			{
				UWjWorldGameRuleJumpMap* GameRule = GameMode->GetCurrentGameRule<UWjWorldGameRuleJumpMap>();
				if (GameRule)
				{
					GameRule->OnPlayerDied(Character);
				}
			}
		}
		UE_LOG(LogWjWorld, Log, TEXT("JumpMapRotatingObstacle: Killed player [%s]"), *Character->GetName());
	}
	else
	{
		// 넉백 방향: 장애물 중심에서 캐릭터 방향
		FVector KnockbackDir = (Character->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		KnockbackDir.Z = FMath::Max(KnockbackDir.Z, 0.3f); // 약간의 상향 넉백
		KnockbackDir.Normalize();

		Character->LaunchCharacter(KnockbackDir * KnockbackForce, true, true);
		UE_LOG(LogWjWorld, Log, TEXT("JumpMapRotatingObstacle: Knocked back player [%s]"), *Character->GetName());
	}
}
