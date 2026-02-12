// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Interact/TeleportToSpawnVolume.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "WjWorldLogCategories.h"

ATeleportToSpawnVolume::ATeleportToSpawnVolume()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(500.f, 500.f, 200.f));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ATeleportToSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATeleportToSpawnVolume::OnTriggerOverlap);
	}
}

void ATeleportToSpawnVolume::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;

	AController* Controller = Character->GetController();
	if (!Controller) return;

	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!GameMode) return;

	AActor* PlayerStart = GameMode->FindPlayerStart(Controller);
	if (!PlayerStart) return;

	const FVector Destination = PlayerStart->GetActorLocation();
	const FRotator Rotation = PlayerStart->GetActorRotation();

	Character->TeleportTo(Destination, Rotation);

	UE_LOG(LogWjWorld, Log, TEXT("TeleportToSpawnVolume: [%s] teleported to PlayerStart"), *Character->GetName());
}
