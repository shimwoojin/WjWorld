// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldBrickMovement.h"
#include "GamePlay/Wall/WjWorldWallManager.h"

#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"
#include "Components/BoxComponent.h"

UWjWorldBrickComponent::UWjWorldBrickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWjWorldBrickComponent::InitializeBrick(const FWjWorldBrickProperties& InBrickProperties)
{
	if (GetOwnerRole() != ROLE_Authority) return;

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

	// 충돌 컴포넌트 크기 설정 (벽돌 크기보다 약간 작게)
	if (BrickCollisionComponent)
	{
		FVector CollisionExtent = InBrickProperties.Size * 0.45f; // 약간 작게
		BrickCollisionComponent->SetBoxExtent(CollisionExtent);
	}

	switch (InBrickProperties.BrickMoveType)
	{
		case EWjWorldBrickMoveType::Standard:
		{
			BrickMovement = NewObject<UWjWorldBrickMovement>(this, UWjWorldBrickMovement::StaticClass());
			BrickMovement->Initialize(this, Cast<UWjWorldGameRuleApproachingWall>(GetGameModePlay()->GetCurrentGameRule()));
			break;
		}
	}
}

void UWjWorldBrickComponent::ReserveDestroyBrick(float AfterSeconds)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this]() {
			if (AActor* OwnerActor = GetOwner())
			{
				OwnerActor->Destroy();
			}
			}, AfterSeconds, false);
	}
}

// Called when the game starts
void UWjWorldBrickComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetGameModePlay()->OnGameLevelChange.AddUObject(this, &UWjWorldBrickComponent::OnBrickMovementSignal);
	}
}

void UWjWorldBrickComponent::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetGameModePlay()->OnGameLevelChange.RemoveAll(this);
		GetWorld()->GetTimerManager().ClearTimer(DestroyHandle);
	}
}

void UWjWorldBrickComponent::OnRegister()
{
	Super::OnRegister();

	BrickMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("BrickMeshComponent"));
	BrickMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	BrickMeshComponent->RegisterComponent();
	BrickMeshComponent->SetIsReplicated(true);

	// 충돌 감지용 박스 컴포넌트 생성
	BrickCollisionComponent = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("BrickCollisionComponent"));
	BrickCollisionComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	BrickCollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BrickCollisionComponent->SetGenerateOverlapEvents(true);
	BrickCollisionComponent->RegisterComponent();

	// 오버랩 이벤트 바인딩
	BrickCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UWjWorldBrickComponent::OnBrickOverlapBegin);
}


// Called every frame
void UWjWorldBrickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_Authority)
	{
		if (BrickMovement)
		{
			BrickMovement->Tick(DeltaTime);
		}
	}
}

void UWjWorldBrickComponent::OnBrickMovementSignal(int32 BrickMoveSignalCount)
{
	if (BrickMovement)
	{
		CurrentBrickMoveSignalCount = BrickMoveSignalCount;
		BrickMovement->MoveBrick(UWjWorldWallManager::GetBrickMovementAllowedTime(BrickMoveSignalCount));
	}
}

void UWjWorldBrickComponent::OnBrickOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 처리
	if (GetOwnerRole() != ROLE_Authority) return;

	// 게임이 시작된 후에만 충돌 처리 (게임 시작 전에는 무시)
	AWjWorldGameModePlay* GameModePlay = GetGameModePlay();
	if (!GameModePlay) return;

	UWjWorldGameRuleApproachingWall* GameRule = Cast<UWjWorldGameRuleApproachingWall>(GameModePlay->GetCurrentGameRule());
	if (!GameRule) return;

	// 플레이어 캐릭터와 충돌했는지 확인
	AWjWorldCharacterPlay* PlayerCharacter = Cast<AWjWorldCharacterPlay>(OtherActor);
	if (PlayerCharacter && !PlayerCharacter->IsEliminated())
	{
		// GameRule에 플레이어 제거 알림
		GameRule->OnPlayerEliminated(PlayerCharacter);
	}
}
