// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/Wall/WjWorldBrickPreviewActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "WjWorldBrickComponent.h"

AWjWorldBrickPreviewActor::AWjWorldBrickPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshComponent"));
	RootComponent = PreviewMeshComponent;

	// 충돌 비활성화
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 기본 큐브 메시 설정
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(UWjWorldBrickComponent::BrickMeshPath);
	if (CubeMesh.Succeeded())
	{
		PreviewMeshComponent->SetStaticMesh(CubeMesh.Object);
	}
}

void AWjWorldBrickPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	// 동적 머티리얼 인스턴스 생성
	if (PreviewMeshComponent)
	{
		DynamicMaterial = PreviewMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
		if (DynamicMaterial)
		{
			// 반투명 설정
			DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), PreviewOpacity);
			SetPreviewValid(true);
		}
	}
}

void AWjWorldBrickPreviewActor::InitializePreview(const FVector& InSize)
{
	// 기본 큐브는 100x100x100이므로 스케일 조정
	FVector Scale = InSize / 100.0f;
	PreviewMeshComponent->SetWorldScale3D(Scale);
}

void AWjWorldBrickPreviewActor::UpdatePreviewLocation(const FVector& InLocation)
{
	SetActorLocation(InLocation);
}

void AWjWorldBrickPreviewActor::SetPreviewValid(bool bInIsValid)
{
	bIsValid = bInIsValid;

	if (DynamicMaterial)
	{
		FLinearColor Color = bIsValid ? ValidColor : InvalidColor;
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}
