// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickMovement.h"

UWjWorldBrickComponent::UWjWorldBrickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWjWorldBrickComponent::InitializeBrick(const FWjWorldBrickProperties& InBrickProperties)
{
	this->BrickProperties = InBrickProperties;

	if (BrickMeshComponent)
	{
		// Set mesh, size, and color based on BrickProperties
		// This is a placeholder; actual implementation would depend on available assets and materials
		FVector Scale = InBrickProperties.Size / 100.0f; // Assuming the default mesh size is 100 units
		BrickMeshComponent->SetWorldScale3D(Scale);

		UStaticMesh* BrickMesh = LoadObject<UStaticMesh>(this, BrickMeshPath);
		if(BrickMesh)
		{
			BrickMeshComponent->SetStaticMesh(BrickMesh);

			UMaterialInstanceDynamic* DynamicMaterial = BrickMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor(InBrickProperties.Color));
			}
		}
	}

	switch (InBrickProperties.BrickMoveType)
	{
		case EWjWorldBrickMoveType::Standard:
		{
			BrickMovement = NewObject<UWjWorldBrickMovement>(this, UWjWorldBrickMovement::StaticClass());
			break;
		}
	}
}

// Called when the game starts
void UWjWorldBrickComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UWjWorldBrickComponent::OnRegister()
{
	Super::OnRegister();

	BrickMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("BrickMeshComponent"));
	BrickMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	BrickMeshComponent->RegisterComponent();
	BrickMeshComponent->SetIsReplicated(true);
}


// Called every frame
void UWjWorldBrickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

