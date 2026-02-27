// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapPushWindActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AJumpMapPushWindActor::AJumpMapPushWindActor()
{
	JumpMapObjectId = TEXT("PushWind");
	PrimaryActorTick.bCanEverTick = true;

	WindZone = CreateDefaultSubobject<UBoxComponent>(TEXT("WindZone"));
	WindZone->SetupAttachment(RootComp);
	WindZone->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	WindZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	WindZone->SetGenerateOverlapEvents(true);
}

void AJumpMapPushWindActor::GetSerializableProperties(TMap<FString, FString>& OutProperties) const
{
	OutProperties.Add(TEXT("WindForce"), FString::SanitizeFloat(WindForce));
	const FVector Extent = WindZone->GetUnscaledBoxExtent();
	OutProperties.Add(TEXT("BoxExtent"), FString::Printf(TEXT("%.2f;%.2f;%.2f"), Extent.X, Extent.Y, Extent.Z));
}

void AJumpMapPushWindActor::ApplySerializedProperties(const TMap<FString, FString>& Properties)
{
	if (const FString* Value = Properties.Find(TEXT("WindForce")))
	{
		WindForce = FCString::Atof(**Value);
	}
	if (const FString* Value = Properties.Find(TEXT("BoxExtent")))
	{
		TArray<FString> Components;
		Value->ParseIntoArray(Components, TEXT(";"), true);
		if (Components.Num() >= 3)
		{
			WindZone->SetBoxExtent(FVector(FCString::Atof(*Components[0]), FCString::Atof(*Components[1]), FCString::Atof(*Components[2])));
		}
	}
}

void AJumpMapPushWindActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	TArray<AActor*> OverlappingActors;
	WindZone->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	const FVector WindDirection = GetActorForwardVector();

	for (AActor* Actor : OverlappingActors)
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character) continue;

		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (!MovementComp) continue;

		// 매 프레임 바람 방향으로 속도 추가
		Character->LaunchCharacter(WindDirection * WindForce * DeltaTime, false, false);
	}
}
