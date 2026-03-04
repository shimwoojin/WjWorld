// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapGrapplePointActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

AJumpMapGrapplePointActor::AJumpMapGrapplePointActor()
{
	JumpMapObjectId = TEXT("GrapplePoint");
	GrappleRange = CreateDefaultSubobject<USphereComponent>(TEXT("GrappleRange"));
	GrappleRange->SetupAttachment(RootComp);
	GrappleRange->SetSphereRadius(GrappleRadius);
	GrappleRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GrappleRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	GrappleRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GrappleRange->SetGenerateOverlapEvents(true);
	GrappleRange->SetHiddenInGame(false);

	// 그래플 포인트 메시는 작은 구체로 표시
	MeshComponent->SetRelativeScale3D(FVector(0.3f));
}

void AJumpMapGrapplePointActor::BeginPlay()
{
	Super::BeginPlay();

	GrappleRange->OnComponentBeginOverlap.AddDynamic(this, &AJumpMapGrapplePointActor::OnGrappleRangeBeginOverlap);
	GrappleRange->OnComponentEndOverlap.AddDynamic(this, &AJumpMapGrapplePointActor::OnGrappleRangeEndOverlap);
}

void AJumpMapGrapplePointActor::GetSerializableProperties(TMap<FString, FString>& OutProperties) const
{
	OutProperties.Add(TEXT("GrappleRadius"), FString::SanitizeFloat(GrappleRadius));
}

void AJumpMapGrapplePointActor::ApplySerializedProperties(const TMap<FString, FString>& Properties)
{
	if (const FString* Value = Properties.Find(TEXT("GrappleRadius")))
	{
		GrappleRadius = FCString::Atof(**Value);
		if (GrappleRange)
		{
			GrappleRange->SetSphereRadius(GrappleRadius);
		}
	}
}

bool AJumpMapGrapplePointActor::IsInRange(const FVector& FromLocation) const
{
	const float DistSq = FVector::DistSquared(FromLocation, GetActorLocation());
	return DistSq <= (GrappleRadius * GrappleRadius);
}

FVector AJumpMapGrapplePointActor::GetGrappleTargetLocation() const
{
	return GetActorLocation();
}

void AJumpMapGrapplePointActor::OnGrappleRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (Character->IsLocallyControlled())
		{
			MeshComponent->SetRenderCustomDepth(true);
			MeshComponent->SetCustomDepthStencilValue(252);
			MeshComponent->SetRelativeScale3D(FVector(0.5f));
		}
	}
}

void AJumpMapGrapplePointActor::OnGrappleRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (Character->IsLocallyControlled())
		{
			MeshComponent->SetRenderCustomDepth(false);
			MeshComponent->SetRelativeScale3D(FVector(0.3f));
		}
	}
}
