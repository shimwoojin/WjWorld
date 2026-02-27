// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapKillZoneActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleJumpMap.h"
#include "WjWorldLogCategories.h"

AJumpMapKillZoneActor::AJumpMapKillZoneActor()
{
	JumpMapObjectId = TEXT("KillZone");
	KillTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("KillTrigger"));
	KillTrigger->SetupAttachment(RootComp);
	KillTrigger->SetBoxExtent(FVector(500.f, 500.f, 50.f));
	KillTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	KillTrigger->SetGenerateOverlapEvents(true);

	// 킬존은 보이지 않아야 함
	MeshComponent->SetVisibility(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AJumpMapKillZoneActor::GetSerializableProperties(TMap<FString, FString>& OutProperties) const
{
	const FVector Extent = KillTrigger->GetUnscaledBoxExtent();
	OutProperties.Add(TEXT("BoxExtent"), FString::Printf(TEXT("%.2f;%.2f;%.2f"), Extent.X, Extent.Y, Extent.Z));
}

void AJumpMapKillZoneActor::ApplySerializedProperties(const TMap<FString, FString>& Properties)
{
	if (const FString* Value = Properties.Find(TEXT("BoxExtent")))
	{
		TArray<FString> Components;
		Value->ParseIntoArray(Components, TEXT(";"), true);
		if (Components.Num() >= 3)
		{
			KillTrigger->SetBoxExtent(FVector(FCString::Atof(*Components[0]), FCString::Atof(*Components[1]), FCString::Atof(*Components[2])));
		}
	}
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
