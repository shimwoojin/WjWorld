// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Sumo/SumoPowerUpActor.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"
#include "WjWorldGameplayTag.h"
#include "WjWorldLogCategories.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASumoPowerUpActor::ASumoPowerUpActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Collision
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(80.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionSphere;

	// Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.5f));

	// Rotating
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.f, 90.f, 0.f);
}

void ASumoPowerUpActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASumoPowerUpActor::OnOverlapBegin);
	}
}

void ASumoPowerUpActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(OtherActor);
	if (!Character || Character->IsEliminated()) return;

	ApplyPowerUp(Character);
	Destroy();
}

void ASumoPowerUpActor::ApplyPowerUp(AWjWorldCharacterPlay* Character)
{
	if (!Character) return;

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC) return;

	switch (PowerUpType)
	{
	case ESumoPowerUpType::SpeedBoost:
	{
		ASC->AddLooseGameplayTag(WjWorldGameplayTag::Buff_SpeedBoost());

		// 이동속도 직접 수정
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp)
		{
			float OriginalSpeed = MovementComp->MaxWalkSpeed;
			MovementComp->MaxWalkSpeed *= SpeedBoostMultiplier;

			// 지속 시간 후 복원
			TWeakObjectPtr<AWjWorldCharacterPlay> WeakChar = Character;
			float RestoreSpeed = OriginalSpeed;
			FTimerHandle SpeedTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(SpeedTimerHandle, FTimerDelegate::CreateLambda(
				[WeakChar, RestoreSpeed]()
				{
					if (AWjWorldCharacterPlay* Char = WeakChar.Get())
					{
						if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
						{
							MC->MaxWalkSpeed = RestoreSpeed;
						}
						if (UAbilitySystemComponent* CharASC = Char->GetAbilitySystemComponent())
						{
							CharASC->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_SpeedBoost());
						}
					}
				}), SpeedBoostDuration, false);
		}

		UE_LOG(LogWjWorld, Log, TEXT("SumoPowerUp: SpeedBoost applied to %s (%.1fs)"),
			*Character->GetName(), SpeedBoostDuration);
		break;
	}

	case ESumoPowerUpType::SuperPush:
	{
		ASC->AddLooseGameplayTag(WjWorldGameplayTag::Buff_SuperPush());
		UE_LOG(LogWjWorld, Log, TEXT("SumoPowerUp: SuperPush applied to %s"), *Character->GetName());
		break;
	}

	case ESumoPowerUpType::Shield:
	{
		ASC->AddLooseGameplayTag(WjWorldGameplayTag::Buff_Shield());
		UE_LOG(LogWjWorld, Log, TEXT("SumoPowerUp: Shield applied to %s"), *Character->GetName());
		break;
	}
	}

	// GameplayCue 실행
	FGameplayCueParameters CueParams;
	CueParams.Location = Character->GetActorLocation();
	ASC->ExecuteGameplayCue(WjWorldGameplayTag::GameplayCue_Sumo_PowerUp_Pickup(), CueParams);
}
