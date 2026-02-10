// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapGrapplePointActor.h"
#include "Components/SphereComponent.h"

AJumpMapGrapplePointActor::AJumpMapGrapplePointActor()
{
	GrappleRange = CreateDefaultSubobject<USphereComponent>(TEXT("GrappleRange"));
	GrappleRange->SetupAttachment(RootComp);
	GrappleRange->SetSphereRadius(GrappleRadius);
	GrappleRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrappleRange->SetHiddenInGame(false);

	// 그래플 포인트 메시는 작은 구체로 표시
	MeshComponent->SetRelativeScale3D(FVector(0.3f));
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
