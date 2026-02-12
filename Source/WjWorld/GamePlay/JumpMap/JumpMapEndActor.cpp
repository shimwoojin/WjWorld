// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapEndActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleJumpMap.h"
#include "WjWorldLogCategories.h"

AJumpMapEndActor::AJumpMapEndActor()
{
	JumpMapObjectId = TEXT("End");
	FinishTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("FinishTrigger"));
	FinishTrigger->SetupAttachment(RootComp);
	FinishTrigger->SetBoxExtent(FVector(100.f, 100.f, 200.f));
	FinishTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	FinishTrigger->SetGenerateOverlapEvents(true);
}

void AJumpMapEndActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FinishTrigger->OnComponentBeginOverlap.AddDynamic(this, &AJumpMapEndActor::OnFinishOverlap);
	}
}

void AJumpMapEndActor::OnFinishOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(OtherActor);
	if (!Character || Character->IsEliminated()) return;

	// GameRule에 완주 알림
	UWorld* World = GetWorld();
	if (!World) return;

	AWjWorldGameModePlay* GameMode = Cast<AWjWorldGameModePlay>(World->GetAuthGameMode());
	if (!GameMode) return;

	UWjWorldGameRuleJumpMap* GameRule = GameMode->GetCurrentGameRule<UWjWorldGameRuleJumpMap>();
	if (GameRule)
	{
		GameRule->OnPlayerFinished(Character);
	}

	UE_LOG(LogWjWorld, Log, TEXT("JumpMapEnd: Player [%s] finished the course"), *Character->GetName());
}
