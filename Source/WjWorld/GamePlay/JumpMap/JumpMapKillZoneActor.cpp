// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapKillZoneActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleJumpMap.h"
#include "WjWorldLogCategories.h"

AJumpMapKillZoneActor::AJumpMapKillZoneActor()
{
	KillTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("KillTrigger"));
	KillTrigger->SetupAttachment(RootComp);
	KillTrigger->SetBoxExtent(FVector(500.f, 500.f, 50.f));
	KillTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	KillTrigger->SetGenerateOverlapEvents(true);

	// 킬존은 보이지 않아야 함
	MeshComponent->SetVisibility(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AJumpMapKillZoneActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		KillTrigger->OnComponentBeginOverlap.AddDynamic(this, &AJumpMapKillZoneActor::OnKillOverlap);
	}
}

void AJumpMapKillZoneActor::OnKillOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(OtherActor);
	if (!Character || Character->IsEliminated()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AWjWorldGameModePlay* GameMode = Cast<AWjWorldGameModePlay>(World->GetAuthGameMode());
	if (!GameMode) return;

	UWjWorldGameRuleJumpMap* GameRule = GameMode->GetCurrentGameRule<UWjWorldGameRuleJumpMap>();
	if (GameRule)
	{
		GameRule->OnPlayerDied(Character);
	}

	UE_LOG(LogWjWorld, Log, TEXT("JumpMapKillZone: Player [%s] hit kill zone"), *Character->GetName());
}
