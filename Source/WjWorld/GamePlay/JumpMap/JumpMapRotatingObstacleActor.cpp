// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapRotatingObstacleActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleJumpMap.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "WjWorldLogCategories.h"

AJumpMapRotatingObstacleActor::AJumpMapRotatingObstacleActor()
{
	JumpMapObjectId = TEXT("RotatingObstacle");
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	HitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("HitTrigger"));
	HitTrigger->SetupAttachment(RootComp);
	HitTrigger->SetBoxExtent(FVector(200.f, 50.f, 50.f));
	HitTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HitTrigger->SetGenerateOverlapEvents(true);
}

void AJumpMapRotatingObstacleActor::GetSerializableProperties(TMap<FString, FString>& OutProperties) const
{
	OutProperties.Add(TEXT("RotationSpeed"), FString::SanitizeFloat(RotationSpeed));
	OutProperties.Add(TEXT("RotationAxis"), FString::Printf(TEXT("%.2f;%.2f;%.2f"), RotationAxis.Pitch, RotationAxis.Yaw, RotationAxis.Roll));
	OutProperties.Add(TEXT("bKillOnHit"), bKillOnHit ? TEXT("1") : TEXT("0"));
	OutProperties.Add(TEXT("KnockbackForce"), FString::SanitizeFloat(KnockbackForce));
}

void AJumpMapRotatingObstacleActor::ApplySerializedProperties(const TMap<FString, FString>& Properties)
{
	if (const FString* Value = Properties.Find(TEXT("RotationSpeed")))
	{
		RotationSpeed = FCString::Atof(**Value);
	}
	if (const FString* Value = Properties.Find(TEXT("RotationAxis")))
	{
		TArray<FString> Components;
		Value->ParseIntoArray(Components, TEXT(";"), true);
		if (Components.Num() >= 3)
		{
			RotationAxis = FRotator(FCString::Atof(*Components[0]), FCString::Atof(*Components[1]), FCString::Atof(*Components[2]));
		}
	}
	if (const FString* Value = Properties.Find(TEXT("bKillOnHit")))
	{
		bKillOnHit = (*Value == TEXT("1"));
	}
	if (const FString* Value = Properties.Find(TEXT("KnockbackForce")))
	{
		KnockbackForce = FCString::Atof(**Value);
	}
}

void AJumpMapRotatingObstacleActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJumpMapRotatingObstacleActor, ServerElapsedTime);
}

void AJumpMapRotatingObstacleActor::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = GetActorRotation();

	if (HasAuthority())
	{
		HitTrigger->OnComponentBeginOverlap.AddDynamic(this, &AJumpMapRotatingObstacleActor::OnHitOverlap);
	}
}

void AJumpMapRotatingObstacleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// 서버: 경과 시간 누적
		ServerElapsedTime += DeltaTime;
	}

	// 서버/클라이언트 모두 동일한 시간 기반 회전 계산 (누적 오차 없음)
	const FRotator TotalRotation = RotationAxis * RotationSpeed * ServerElapsedTime;
	SetActorRotation(InitialRotation + TotalRotation);
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
