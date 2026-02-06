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
#include "Net/UnrealNetwork.h"

// Static 멤버 정의
TMap<TWeakObjectPtr<AWjWorldCharacterPlay>, FTimerHandle> ASumoPowerUpActor::SpeedBoostTimerHandles;

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

	// 초기 메시 적용 (서버에서 이미 타입이 설정된 경우)
	ApplyMeshForType();
}

void ASumoPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASumoPowerUpActor, PowerUpType);
}

void ASumoPowerUpActor::SetPowerUpType(ESumoPowerUpType NewType)
{
	PowerUpType = NewType;
	ApplyMeshForType();
}

void ASumoPowerUpActor::OnRep_PowerUpType()
{
	ApplyMeshForType();
}

void ASumoPowerUpActor::ApplyMeshForType()
{
	if (!Mesh) return;

	UStaticMesh* TargetMesh = nullptr;

	switch (PowerUpType)
	{
	case ESumoPowerUpType::SpeedBoost:
		TargetMesh = SpeedBoostMesh;
		break;
	case ESumoPowerUpType::SuperPush:
		TargetMesh = SuperPushMesh;
		break;
	case ESumoPowerUpType::Shield:
		TargetMesh = ShieldMesh;
		break;
	}

	if (TargetMesh)
	{
		Mesh->SetStaticMesh(TargetMesh);
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
		// 기존 SpeedBoost 타이머가 있으면 취소하고 속도 먼저 복원
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		TWeakObjectPtr<AWjWorldCharacterPlay> WeakChar = Character;

		if (FTimerHandle* ExistingHandle = SpeedBoostTimerHandles.Find(WeakChar))
		{
			if (ExistingHandle->IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(*ExistingHandle);
			}
			SpeedBoostTimerHandles.Remove(WeakChar);
		}

		ASC->AddLooseGameplayTag(WjWorldGameplayTag::Buff_SpeedBoost());

		// 이동속도 직접 수정
		if (MovementComp)
		{
			// 이미 버프가 적용된 상태가 아니라면 원래 속도에서 배율 적용
			// (태그로 확인하므로 기존 타이머 취소 후에도 속도가 이미 빨라진 상태일 수 있음)
			float BaseSpeed = MovementComp->MaxWalkSpeed;
			if (!ASC->HasMatchingGameplayTag(WjWorldGameplayTag::Buff_SpeedBoost()))
			{
				// 태그가 방금 추가되었으므로 이 경우는 해당 안됨, 하지만 안전을 위해 체크
				BaseSpeed = MovementComp->MaxWalkSpeed;
			}
			MovementComp->MaxWalkSpeed = BaseSpeed * SpeedBoostMultiplier;

			float RestoreSpeed = BaseSpeed;

			// 지속 시간 후 복원
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
					// 타이머 완료 후 맵에서 제거
					SpeedBoostTimerHandles.Remove(WeakChar);
				}), SpeedBoostDuration, false);

			// 타이머 핸들 저장
			SpeedBoostTimerHandles.Add(WeakChar, SpeedTimerHandle);
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
