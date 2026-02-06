// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldTileActor.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/WjWorldGameplayUtils.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "WjWorldLogCategories.h"

#include "Net/UnrealNetwork.h"

const FColor AWjWorldTileActor::BombSignalOnColorWarning = FColor::Yellow;
const FColor AWjWorldTileActor::BombSignalOnColorDanger = FColor::Red;

UStaticMesh* AWjWorldTileActor::GetTileMesh()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings && !Settings->TileMesh.IsNull())
	{
		return Settings->TileMesh.LoadSynchronous();
	}
	return nullptr;
}

AWjWorldTileActor::AWjWorldTileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CenterHitBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CenterHitBoxComponent"));
	SetRootComponent(CenterHitBoxComponent);
	CenterHitBoxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CenterHitBoxComponent->SetBoxExtent(FVector(HitBoxSize, HitBoxSize, HitBoxSize));

	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		HitBoxComponents[DirectionIndex] = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("HitBoxComponent_%d"), DirectionIndex));
		HitBoxComponents[DirectionIndex]->SetupAttachment(RootComponent);
		HitBoxComponents[DirectionIndex]->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		HitBoxComponents[DirectionIndex]->SetBoxExtent(FVector(HitBoxSize, HitBoxSize, HitBoxSize));
	}

	TileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMeshComponent"));
	TileMeshComponent->SetupAttachment(RootComponent);

	bIsBombSignalOn = false;
}

void AWjWorldTileActor::InitializeTile(const FVector& InSize, const FVector& InCenterOffset, bool bInIsWhiteTile)
{
	if (!HasAuthority()) return;

	FVector TileLocation = InCenterOffset;
	TileLocation.Z -= InSize.Z * 0.5f; // 타일이 바닥에 위치하도록 Z 오프셋 조정
	SetActorLocation(TileLocation);
	UE_LOG(LogWjWorld, Log, TEXT("Tile Initialized at Location: %s"), *InCenterOffset.ToString());

	// FinishSpawning 전에 설정하여 Initial Bunch에 포함
	bIsWhiteTile = bInIsWhiteTile;

	float Offset = HitBoxSize + 2.0f;

	// ⭐ InSize는 타일 전체 크기, HalfExtent는 절반
	const FVector HalfExtent = InSize * 0.5f;

	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		if (HitBoxComponents[DirectionIndex] == nullptr) continue;

		FVector BoxLocation = FVector::ZeroVector;

		// 방향별 HitBox는 타일 경계 끝에 배치 (half extent 기준)
		switch (DirectionIndex)
		{
		case EWjWorldDirection::Up:
		{
			BoxLocation = FVector(0.0f, HalfExtent.Y, 0.0f);
			break;
		}

		case EWjWorldDirection::Right:
		{
			BoxLocation = FVector(HalfExtent.X, 0.0f, 0.0f);
			break;
		}

		case EWjWorldDirection::Down:
		{
			BoxLocation = FVector(0.0f, -HalfExtent.Y, 0.0f);
			break;
		}

		case EWjWorldDirection::Left:
		{
			BoxLocation = FVector(-HalfExtent.X, 0, 0.0f);
			break;
		}
		}

		HitBoxComponents[DirectionIndex]->SetRelativeLocation(BoxLocation);
		HitBoxComponents[DirectionIndex]->OnComponentBeginOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapBegin);
		HitBoxComponents[DirectionIndex]->OnComponentEndOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapEnd);
	}

	// ⭐ SetBoxExtent는 half extent를 받으므로 InSize * 0.5f 전달
	CenterHitBoxComponent->SetBoxExtent(HalfExtent);
	CenterHitBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapBegin);
	CenterHitBoxComponent->OnComponentEndOverlap.AddDynamic(this, &AWjWorldTileActor::OnBrickOverlapEnd);

	if (TileMeshComponent)
	{
		TileMeshComponent->SetWorldScale3D(InSize / 100.0f); // 기본 메시 크기가 100이므로 스케일 조정
	}
}

void AWjWorldTileActor::PostNetInit()
{
	Super::PostNetInit();

	// 클라이언트에서 첫 리플리케이션 후 색상 적용
	DefaultBaseColor = bIsWhiteTile ? FLinearColor::White : FLinearColor::Black;
	ApplyTileColor();

	UE_LOG(LogWjWorld, Log, TEXT("Tile PostNetInit on Client: IsWhiteTile=%s"), bIsWhiteTile ? TEXT("True") : TEXT("False"));
}

// Called when the game starts or when spawned
void AWjWorldTileActor::BeginPlay()
{
	Super::BeginPlay();

	// DeveloperSettings에서 메시 로드
	if (TileMeshComponent && !TileMeshComponent->GetStaticMesh())
	{
		UStaticMesh* Mesh = GetTileMesh();
		if (Mesh)
		{
			TileMeshComponent->SetStaticMesh(Mesh);
		}
	}

	if (HasAuthority())
	{
		AWjWorldGameModePlay* GameModePlay = GetWorld()->GetAuthGameMode<AWjWorldGameModePlay>();
		if (GameModePlay)
		{
			GameRule = GameModePlay->GetCurrentGameRule<UWjWorldGameRuleApproachingWall>();
		}
	}

	// 동적 머티리얼 인스턴스 생성
	if (!DynamicMaterial && TileMeshComponent)
	{
		UMaterialInterface* BaseMaterial = TileMeshComponent->GetMaterial(0);
		if (BaseMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			TileMeshComponent->SetMaterial(0, DynamicMaterial);
		}
	}

	// 서버에서 색상 적용
	if (HasAuthority())
	{
		DefaultBaseColor = bIsWhiteTile ? FLinearColor::White : FLinearColor::Black;
		ApplyTileColor();
	}
}

// Called every frame
void AWjWorldTileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsBombSignalOn == false) return;

	ElapsedBombingTime += DeltaTime;

	// Warning -> Danger 색상 보간
	if (DynamicMaterial)
	{
		const float Alpha = FMath::Clamp(ElapsedBombingTime / BombChargingTime, 0.0f, 1.0f);
		const FLinearColor WarningColor = FLinearColor(BombSignalOnColorWarning);
		const FLinearColor DangerColor = FLinearColor(BombSignalOnColorDanger);
		const FLinearColor LerpedColor = FMath::Lerp(WarningColor, DangerColor, Alpha);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FVector(LerpedColor.R, LerpedColor.G, LerpedColor.B));
	}

	if (ElapsedBombingTime >= BombChargingTime)
	{
		if (HasAuthority())
		{
			Bomb();
			bIsBombSignalOn = false;
			OnRep_IsBombSignalOn();
		}

		ElapsedBombingTime = 0.0f;
	}
}

void AWjWorldTileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWjWorldTileActor, bIsBombSignalOn);
	DOREPLIFETIME_CONDITION(AWjWorldTileActor, bIsWhiteTile, COND_InitialOnly);
}

void AWjWorldTileActor::OnBrickOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 처리
	if (GetLocalRole() != ROLE_Authority) return;
	
	AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(OtherActor);
	if (!BrickActor) return;

	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		if (HitBoxComponents[DirectionIndex] == OverlappedComponent)
		{
			++bIsOverlapBricks[DirectionIndex];
		}
	}

	if (WjWorldGameplayUtils::IsGameplayPhaseplaying(GetWorld()))
	{
		if (bIsBombSignalOn == false && CheckBombSignalOn())
		{
			bIsBombSignalOn = true;
			OnRep_IsBombSignalOn();
		}
	}
}

void AWjWorldTileActor::OnBrickOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetLocalRole() != ROLE_Authority) return;

	AWjWorldBrickActor* BrickActor = Cast<AWjWorldBrickActor>(OtherActor);
	if (!BrickActor) return;

	if (OverlappedComponent == CenterHitBoxComponent)
	{
		const FWjWorldBrickProperties& BrickProperties = BrickActor->GetBrickComponent()->GetBrickProperties();
		if (BrickProperties.BrickType == EWjWorldBrickType::Standard)
		{
			Destroy();
			return;
		}
	}
	else
	{
		for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
		{
			if (HitBoxComponents[DirectionIndex] == OverlappedComponent)
			{
				--bIsOverlapBricks[DirectionIndex];
				bIsOverlapBricks[DirectionIndex] = FMath::Max(0, bIsOverlapBricks[DirectionIndex]);
			}
		}
	}

	if (WjWorldGameplayUtils::IsGameplayPhaseplaying(GetWorld()))
	{
		if (bIsBombSignalOn && !CheckBombSignalOn())
		{
			bIsBombSignalOn = false;
			OnRep_IsBombSignalOn();
		}
	}
}

bool AWjWorldTileActor::CheckBombSignalOn() const
{
	for (int32 DirectionIndex = 0; DirectionIndex < EWjWorldDirection::Max; ++DirectionIndex)
	{
		if (bIsOverlapBricks[DirectionIndex] <= 0)
		{
			return false;
		}
	}

	return true;
}

void AWjWorldTileActor::Bomb()
{
	if (GetLocalRole() != ROLE_Authority) return;
	if (WjWorldGameplayUtils::IsGameplayPhaseplaying(GetWorld()) == false) return;

	TArray<AActor*> BombedActors;
	CenterHitBoxComponent->GetOverlappingActors(BombedActors, AWjWorldCharacterPlay::StaticClass());

	// 타일의 실제 경계 계산 (옆 칸에 있는 캐릭터 제외)
	const FVector TileCenter = GetActorLocation();
	const FVector TileExtent = CenterHitBoxComponent->GetScaledBoxExtent();
	// 경계를 약간 줄여서 타일 경계에 선 캐릭터가 양쪽에서 처리되지 않도록 함
	const float BoundaryMargin = 5.0f; // 5cm 마진
	const FBox TileBounds(TileCenter - TileExtent + FVector(BoundaryMargin), TileCenter + TileExtent - FVector(BoundaryMargin));

	for (AActor* BombedActor : BombedActors)
	{
		AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(BombedActor);
		if (Character == nullptr) continue;

		// 캐릭터의 중심점이 이 타일 경계 내에 있는지 확인
		const FVector CharacterLocation = Character->GetActorLocation();
		if (!TileBounds.IsInsideXY(CharacterLocation))
		{
			UE_LOG(LogWjWorld, Verbose, TEXT("AWjWorldTileActor::Bomb - Character at %s is outside tile bounds, skipping"),
				*CharacterLocation.ToString());
			continue;
		}

		if (GameRule.IsValid())
		{
			GameRule->OnPlayerEliminated(Character);
		}
	}

	SpawnBombEffect();
}

void AWjWorldTileActor::OnRep_IsBombSignalOn()
{
	ElapsedBombingTime = 0.0f;

	// 폭탄 신호가 꺼지면 기본 색상으로 복원
	if (!bIsBombSignalOn && DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DefaultBaseColor);
	}

	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldTileActor::OnRep_IsBombSignalOn: %d"), bIsBombSignalOn);
}

void AWjWorldTileActor::SpawnBombEffect_Implementation()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		NiagaraSystem,  // UNiagaraSystem*
		GetActorLocation(),
		GetActorRotation(),
		GetActorScale3D(),  // Scale
		true,           // Auto Destroy
		true,           // Auto Activate
		ENCPoolMethod::None
	);

	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldTileActor::SpawnBombEffect_Implementation"));
}

void AWjWorldTileActor::ApplyTileColor()
{
	// DynamicMaterial이 아직 없으면 생성 (BeginPlay 전에 RPC가 도착한 경우)
	if (!DynamicMaterial && TileMeshComponent)
	{
		UMaterialInterface* BaseMaterial = TileMeshComponent->GetMaterial(0);
		if (BaseMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			TileMeshComponent->SetMaterial(0, DynamicMaterial);
		}
	}

	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DefaultBaseColor);
	}
}
