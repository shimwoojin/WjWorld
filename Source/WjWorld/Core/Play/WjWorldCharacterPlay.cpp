// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "AbilitySystem/WjWorldAbilitySystemComponent.h"
#include "Cosmetic/WjWorldCosmeticComponent.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "DataAsset/CharacterPlaySetupDataAsset.h"
#include "UI/Ability/AbilityPromptWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

#include "WjWorldLogCategories.h"
#include "WjWorldGameplayTag.h"
#include "WjTypes.h"

// SpawnBrick Server RPC용
#include "Core/Play/WjWorldGameModePlay.h"
#include "Core/GameRule/WjWorldGameRuleApproachingWall.h"
#include "GamePlay/Wall/WjWorldBrickSpawner.h"
#include "GamePlay/Wall/WjWorldBrickComponent.h"
#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "AbilitySystem/AttributeSets/WjWorldCharacterAttributeSet.h"
#include "AbilitySystem/Effects/GE_SpawnBrickChargeCost.h"
#include "AbilitySystem/Abilities/GA_LiftBrick.h"
#include "GamePlay/Wall/WjWorldBrickActor.h"
#include "GameplayEffect.h"
#include "GameplayCueManager.h"
#include "Engine/OverlapResult.h"

AWjWorldCharacterPlay::AWjWorldCharacterPlay()
{
	// 어빌리티 프롬프트 WidgetComponent (캐릭터 머리 위에 Screen 공간으로 표시)
	AbilityPromptComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AbilityPromptComponent"));
	AbilityPromptComponent->SetupAttachment(RootComponent);
	AbilityPromptComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	AbilityPromptComponent->SetWidgetSpace(EWidgetSpace::Screen);
	AbilityPromptComponent->SetDrawAtDesiredSize(true);
	AbilityPromptComponent->SetVisibility(false);

	// 들고 있는 벽돌 메시 컴포넌트 (손 소켓에 부착될 예정)
	LiftedBrickMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiftedBrickMeshComponent"));
	LiftedBrickMeshComponent->SetupAttachment(GetMesh(), FName("hand_r"));
	LiftedBrickMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	LiftedBrickMeshComponent->SetRelativeScale3D(FVector(0.3f));
	LiftedBrickMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LiftedBrickMeshComponent->SetVisibility(false);
}

void AWjWorldCharacterPlay::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

UAbilitySystemComponent* AWjWorldCharacterPlay::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

UWjWorldAbilitySystemComponent* AWjWorldCharacterPlay::GetWJAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void AWjWorldCharacterPlay::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWjWorldCharacterPlay, bIsEliminated);
	DOREPLIFETIME(AWjWorldCharacterPlay, bIsCarryingBrick);
	DOREPLIFETIME(AWjWorldCharacterPlay, CarriedBrickMesh);
	DOREPLIFETIME(AWjWorldCharacterPlay, CarriedBrickScale);
	DOREPLIFETIME(AWjWorldCharacterPlay, CarriedBrickColor);
}

void AWjWorldCharacterPlay::OnEliminated()
{
	if (bIsEliminated)
	{
		return; // 이미 사망한 경우 무시
	}

	// Shield 버프 체크 (있으면 소모하고 제거 무시)
	if (AbilitySystemComponent.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(WjWorldGameplayTag::Buff_Shield()))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(WjWorldGameplayTag::Buff_Shield());
		UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::OnEliminated - Shield consumed, elimination prevented"));
		return;
	}

	bIsEliminated = true;

	// ASC에 State.Eliminated 태그 추가 → 어빌리티 활성화 차단
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(WjWorldGameplayTag::State_Eliminated());
		AbilitySystemComponent->CancelAllAbilities();
	}

	// PlayerState 데이터 컴포넌트 업데이트는 각 GameRule에서 처리

	HandleEliminationEffects();
}

void AWjWorldCharacterPlay::OnRep_IsEliminated()
{
	if (bIsEliminated)
	{
		HandleEliminationEffects();
	}
}

void AWjWorldCharacterPlay::HandleEliminationEffects()
{
	// 움직임 비활성화
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->DisableMovement();
		MovementComp->StopMovementImmediately();
	}

	// 관전 대상 전환 (로컬 컨트롤러에서만)
	// DisableInput은 사용하지 않음 — 카메라 Yaw 입력까지 차단되어 관전 시 회전 불가
	// 이동은 이미 위에서 DisableMovement()로 차단, 어빌리티는 State_Eliminated 태그 + CancelAllAbilities로 차단
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		// 살아있는 다른 플레이어 찾아서 관전
		AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();
		if (GS)
		{
			for (APlayerState* PS : GS->PlayerArray)
			{
				if (!PS || PS == PC->PlayerState) continue;
				APawn* OtherPawn = PS->GetPawn();
				AWjWorldCharacterPlay* OtherChar = Cast<AWjWorldCharacterPlay>(OtherPawn);
				if (OtherChar && !OtherChar->IsEliminated())
				{
					PC->SetViewTargetWithBlend(OtherChar, 0.5f);
					PC->SetControlRotation(OtherChar->GetActorRotation());
					UE_LOG(LogWjWorld, Log, TEXT("HandleEliminationEffects: Spectating %s"), *PS->GetPlayerName());
					break;
				}
			}
		}
	}

	// 1초 후 캐릭터 파괴
	if (GetWorld())
	{
		FTimerHandle DestroyHandle;
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this]()
		{
			Destroy();
		}, 1.0f, false);
	}
}

void AWjWorldCharacterPlay::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// ASC 초기화 (CharacterPlay 전용)
	APlayerState* WJPlayerState = GetPlayerState<AWjWorldPlayerStatePlay>();
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(WJPlayerState);
	if (AbilitySystemInterface)
	{
		AbilitySystemComponent = Cast<UWjWorldAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
		if (AbilitySystemComponent.IsValid())
		{
			// 클라이언트에서도 ActorInfo 초기화 필요 (어빌리티 입력 처리를 위해)
			AbilitySystemComponent->InitAbilityActorInfo(WJPlayerState, this);

			// Confirm/Cancel InputID 설정
			AbilitySystemComponent->GenericConfirmInputID = static_cast<int32>(EWjWorldAbilityInputID::Confirm);
			AbilitySystemComponent->GenericCancelInputID = static_cast<int32>(EWjWorldAbilityInputID::Cancel);

			// 클라이언트 공통 초기화 (어빌리티 부여는 Initialize 내부에서 Authority 체크)
			FLoadSoftObjectPathAsyncDelegate SetupDataLoadDel;
			SetupDataLoadDel.BindUObject(this, &ThisClass::OnSetupDataAssetLoaded);
			SetupDataAsset.LoadAsync(SetupDataLoadDel);
		}
	}

	// 코스메틱은 Super::OnRep_PlayerState()에서 처리됨 (CharacterBase)
}

void AWjWorldCharacterPlay::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerState* WJPlayerState = GetPlayerState<AWjWorldPlayerStatePlay>();
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(WJPlayerState);
	if (AbilitySystemInterface)
	{
		AbilitySystemComponent = Cast<UWjWorldAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
		if (!ensureMsgf(AbilitySystemComponent.IsValid(), TEXT("AWjWorldCharacterPlay::PossessedBy - AbilitySystemComponent is invalid")))
		{
			return;
		}
		AbilitySystemComponent->InitAbilityActorInfo(WJPlayerState, this);
		AbilitySystemComponent->SetOwnerActor(NewController);

		// Confirm/Cancel InputID 설정
		AbilitySystemComponent->GenericConfirmInputID = static_cast<int32>(EWjWorldAbilityInputID::Confirm);
		AbilitySystemComponent->GenericCancelInputID = static_cast<int32>(EWjWorldAbilityInputID::Cancel);

		FLoadSoftObjectPathAsyncDelegate SetupDataLoadDel;
		SetupDataLoadDel.BindUObject(this, &ThisClass::OnSetupDataAssetLoaded);
		SetupDataAsset.LoadAsync(SetupDataLoadDel);
	}

	// 코스메틱 로드아웃 설정 (서버에서)
	if (HasAuthority())
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			if (CosmeticComponent)
			{
				CosmeticComponent->SetCatalog(CosmeticSub->GetCatalog());
			}

			if (AWjWorldPlayerStatePlay* PS = GetPlayerState<AWjWorldPlayerStatePlay>())
			{
				// 리스폰 시 PlayerState에 이미 로드아웃이 있으면 덮어쓰지 않음
				// (호스트의 로드아웃으로 다른 플레이어 로드아웃을 덮어쓰는 버그 방지)
				if (PS->GetCosmeticLoadout().Entries.IsEmpty())
				{
					// 최초 스폰: 로컬 서브시스템에서 로드아웃 가져옴 (서버 자신의 캐릭터만 해당)
					APlayerController* PC = Cast<APlayerController>(NewController);
					if (PC && PC->IsLocalController())
					{
						PS->SetCosmeticLoadout(CosmeticSub->GetLoadout());
					}
				}
				// 서버에서도 Pawn 설정 알림 (기존 로드아웃 적용)
				PS->OnPawnSet(nullptr, this);
			}
		}
	}
}

void AWjWorldCharacterPlay::OnSetupDataAssetLoaded(const FSoftObjectPath& Path, UObject* Object)
{
	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::OnSetupDataAssetLoaded"));
	if (!::IsValid(this)) return;

	UCharacterPlaySetupDataAsset* SetupDA = Cast<UCharacterPlaySetupDataAsset>(Object);
	if (SetupDA)
	{
		FString ErrorMsg;
		SetupDA->Initialize(this, ErrorMsg);

		if (ErrorMsg.Len() > 0)
		{
			ensureMsgf(false, TEXT("ErrorMsg : %s"), *ErrorMsg);
		}
		else
		{
			UE_LOG(LogWjWorld, Log, TEXT("SetupDataAsset->Initialize Success"));
		}
	}
}

void AWjWorldCharacterPlay::GasInputPressed(int32 InputID)
{
	Super::GasInputPressed(InputID);
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->PressInputID(InputID);

		// Confirm/Cancel 입력 처리 (WaitConfirmCancel Task용)
		if (InputID == AbilitySystemComponent->GenericConfirmInputID)
		{
			AbilitySystemComponent->LocalInputConfirm();
		}
		else if (InputID == AbilitySystemComponent->GenericCancelInputID)
		{
			AbilitySystemComponent->LocalInputCancel();
		}
	}
}

void AWjWorldCharacterPlay::GasInputReleased(int32 InputID)
{
	Super::GasInputReleased(InputID);
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->ReleaseInputID(InputID);
	}
}

bool AWjWorldCharacterPlay::CanNativeJump() const
{
	return false; // GAS의 GA_Jump가 점프 처리
}

void AWjWorldCharacterPlay::ShowAbilityPrompt(const FText& ConfirmKeyName, const FText& CancelKeyName, const FText& Description)
{
	if (!AbilityPromptComponent)
	{
		return;
	}

	AbilityPromptComponent->SetVisibility(true);

	UAbilityPromptWidget* PromptWidget = Cast<UAbilityPromptWidget>(AbilityPromptComponent->GetWidget());
	if (PromptWidget)
	{
		PromptWidget->SetPromptInfo(ConfirmKeyName, CancelKeyName, Description);
	}
}

void AWjWorldCharacterPlay::HideAbilityPrompt()
{
	if (AbilityPromptComponent)
	{
		AbilityPromptComponent->SetVisibility(false);
	}
}

void AWjWorldCharacterPlay::SetLastAttacker(AWjWorldCharacterPlay* Attacker)
{
	// 자기 자신은 공격자로 설정하지 않음
	if (Attacker && Attacker != this)
	{
		LastAttacker = Attacker;
	}
}

void AWjWorldCharacterPlay::ShowLiftedBrick(UStaticMesh* BrickMesh, const FVector& BrickScale, const FLinearColor& BrickColor)
{
	if (!HasAuthority())
	{
		return;
	}

	CarriedBrickMesh = BrickMesh;
	CarriedBrickScale = BrickScale;
	CarriedBrickColor = BrickColor;
	bIsCarryingBrick = true;

	// 서버에서도 즉시 시각 업데이트
	UpdateLiftedBrickVisual();

	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::ShowLiftedBrick - Mesh: %s, Scale: %s, Color: %s"),
		BrickMesh ? *BrickMesh->GetName() : TEXT("None"),
		*BrickScale.ToString(),
		*BrickColor.ToString());
}

void AWjWorldCharacterPlay::HideLiftedBrick()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsCarryingBrick = false;
	CarriedBrickMesh = nullptr;
	CarriedBrickScale = FVector::OneVector;
	CarriedBrickColor = FLinearColor::White;

	// 서버에서도 즉시 시각 업데이트
	UpdateLiftedBrickVisual();

	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::HideLiftedBrick"));
}

void AWjWorldCharacterPlay::OnRep_IsCarryingBrick()
{
	UpdateLiftedBrickVisual();
}

void AWjWorldCharacterPlay::UpdateLiftedBrickVisual()
{
	if (!LiftedBrickMeshComponent)
	{
		return;
	}

	if (bIsCarryingBrick && CarriedBrickMesh)
	{
		LiftedBrickMeshComponent->SetStaticMesh(CarriedBrickMesh);
		LiftedBrickMeshComponent->SetRelativeScale3D(CarriedBrickScale * 0.3f);
		LiftedBrickMeshComponent->SetVisibility(true);

		// Dynamic Material Instance 생성 및 색상 적용
		if (LiftedBrickMeshComponent->GetNumMaterials() > 0)
		{
			UMaterialInterface* BaseMaterial = LiftedBrickMeshComponent->GetMaterial(0);
			if (BaseMaterial)
			{
				LiftedBrickDynamicMaterial = LiftedBrickMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
				if (LiftedBrickDynamicMaterial)
				{
					LiftedBrickDynamicMaterial->SetVectorParameterValue(FName("BaseColor"), CarriedBrickColor);
				}
			}
		}
	}
	else
	{
		LiftedBrickMeshComponent->SetVisibility(false);
		LiftedBrickMeshComponent->SetStaticMesh(nullptr);
		LiftedBrickDynamicMaterial = nullptr;
	}
}

void AWjWorldCharacterPlay::ServerSpawnBrickAtGridIndex_Implementation(int32 GridX, int32 GridY)
{
	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::ServerSpawnBrickAtGridIndex - GridIndex: (%d, %d)"), GridX, GridY);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 서버에서 WallDesc 가져오기
	FWjWorldWallDescription ServerWallDesc;
	AWjWorldGameModePlay* GameModePlay = World->GetAuthGameMode<AWjWorldGameModePlay>();
	if (GameModePlay)
	{
		UWjWorldGameRuleApproachingWall* GameRule = GameModePlay->GetCurrentGameRule<UWjWorldGameRuleApproachingWall>();
		if (GameRule)
		{
			ServerWallDesc = GameRule->GetWallDesc();
		}
	}

	// WallDesc가 유효하지 않으면 실패
	if (ServerWallDesc.BrickSize.IsZero() || ServerWallDesc.ColumnNum == 0)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("AWjWorldCharacterPlay::ServerSpawnBrickAtGridIndex - Invalid WallDesc"));
		return;
	}

	// 그리드 범위 체크
	if (GridX < 0 || GridX >= ServerWallDesc.ColumnNum ||
		GridY < 0 || GridY >= ServerWallDesc.RowNum)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("AWjWorldCharacterPlay::ServerSpawnBrickAtGridIndex - Invalid GridIndex (%d, %d) for grid %dx%d"),
			GridX, GridY, ServerWallDesc.ColumnNum, ServerWallDesc.RowNum);
		return;
	}

	// 충전 소모 (ASC에서)
	if (AbilitySystemComponent.IsValid())
	{
		const UGameplayEffect* CostGE = UGE_SpawnBrickChargeCost::StaticClass()->GetDefaultObject<UGameplayEffect>();
		if (CostGE)
		{
			FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpec Spec(CostGE, ContextHandle, 1.f);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(Spec);
		}
	}

	FIntPoint SpawnIndexPoint(GridX, GridY);
	FVector SpawnPosition = UWjWorldBrickSpawner::CalculateBrickPosition(
		GridX,
		GridY,
		ServerWallDesc.ColumnNum,
		ServerWallDesc.RowNum,
		ServerWallDesc.CenterOffset,
		ServerWallDesc.BrickSize
	);

	FWjWorldBrickProperties BrickProperties;
	BrickProperties.BrickType = FMath::RandBool() ? EWjWorldBrickType::Moving : EWjWorldBrickType::Destructible;
	BrickProperties.BrickMoveType = EWjWorldBrickMoveType::Standard;
	BrickProperties.Size = ServerWallDesc.BrickSize;
	BrickProperties.Color = BrickProperties.GetColorWithBrickType();
	BrickProperties.SpawnedGridPosition = SpawnIndexPoint;
	BrickProperties.CenterOffset = ServerWallDesc.CenterOffset;
	BrickProperties.ColumnNum = ServerWallDesc.ColumnNum;
	BrickProperties.RowNum = ServerWallDesc.RowNum;

	// Destructible 벽돌은 DeveloperSettings에서 MaxHP 설정
	if (BrickProperties.BrickType == EWjWorldBrickType::Destructible)
	{
		BrickProperties.MaxHP = GetDefault<UWjWorldDeveloperSettings>()->DestructibleBrickDefaultHP;
	}

	UWjWorldBrickSpawner::SpawnBrickActor(World, BrickProperties, GridX, GridY);

	// GameplayCue 실행 (사운드/VFX)
	if (AbilitySystemComponent.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = SpawnPosition;
		AbilitySystemComponent->ExecuteGameplayCue(WjWorldGameplayTag::GameplayCue_Ability_SpawnBrick(), CueParams);
	}
}

void AWjWorldCharacterPlay::ServerLiftBrickAtGridIndex_Implementation(int32 GridX, int32 GridY)
{
	UE_LOG(LogWjWorld, Log, TEXT("AWjWorldCharacterPlay::ServerLiftBrickAtGridIndex - GridIndex: (%d, %d)"), GridX, GridY);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 활성 GA_LiftBrick 어빌리티 인스턴스 찾기
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		UGA_LiftBrick* LiftBrickAbility = Cast<UGA_LiftBrick>(Spec.GetPrimaryInstance());
		if (LiftBrickAbility && LiftBrickAbility->IsActive())
		{
			LiftBrickAbility->ServerHandleBrickPickup(GridX, GridY);
			return;
		}
	}

	UE_LOG(LogWjWorld, Warning, TEXT("AWjWorldCharacterPlay::ServerLiftBrickAtGridIndex - No active GA_LiftBrick found"));
}
