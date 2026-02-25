// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/TreasureChest/WjWorldTreasureChestActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Currency/WjWorldCurrencyTypes.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "UI/Interact/InteractionWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "WjWorldLogCategories.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

const FString AWjWorldTreasureChestActor::ConfigSection = TEXT("TreasureChestCooldown");

AWjWorldTreasureChestActor::AWjWorldTreasureChestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 뚜껑 메시
	LidMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMeshComponent"));
	LidMeshComponent->SetupAttachment(MeshComponent);
	LidMeshComponent->SetRelativeLocation(FVector(0.0f, -60.0f, -60.0f));
	LidMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 상호작용 트리거
	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
	InteractionTrigger->SetupAttachment(MeshComponent);
	InteractionTrigger->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 위젯 컴포넌트
	InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidgetComponent->SetupAttachment(MeshComponent);
	InteractionWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidgetComponent->SetDrawSize(FVector2D(300.0f, 60.0f));
	InteractionWidgetComponent->SetVisibility(false);
}

void AWjWorldTreasureChestActor::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩
	if (InteractionTrigger)
	{
		InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWjWorldTreasureChestActor::OnTriggerBeginOverlap);
		InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AWjWorldTreasureChestActor::OnTriggerEndOverlap);
	}

	// UI 위젯 설정
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings && !Settings->TreasureChestWidgetClass.IsNull())
	{
		UClass* WidgetClass = Settings->TreasureChestWidgetClass.LoadSynchronous();
		if (WidgetClass && InteractionWidgetComponent)
		{
			InteractionWidget = Cast<UInteractionWidget>(CreateWidget<UUserWidget>(GetWorld(), WidgetClass));
			if (InteractionWidget)
			{
				InteractionWidgetComponent->SetWidget(InteractionWidget);
			}
		}
	}

	// GConfig에서 쿨타임 캐시 초기화 (cross-session 복원)
	{
		FString Key = GetChestKey();
		FString LastOpenedStr;
		if (GConfig->GetString(*ConfigSection, *Key, LastOpenedStr, GetConfigPath()))
		{
			FDateTime::ParseIso8601(*LastOpenedStr, CachedLastOpenedTime);
		}
	}

	// 쿨타임 비주얼 초기화
	if (IsOnCooldown())
	{
		ApplyCooldownVisual();
		SetLidOpenImmediate();
	}
	else
	{
		SetLidCloseImmediate();
	}
}

void AWjWorldTreasureChestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 입력 해제 + 바인딩 정리
	if (InteractingPlayer)
	{
		APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController());
		if (PC)
		{
			DisableInput(PC);
		}
	}
	if (InputComponent)
	{
		InputComponent->ClearActionBindings();
	}
	bInputBound = false;

	// 쿨타임 UI 타이머 정리
	GetWorldTimerManager().ClearTimer(CooldownUITimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AWjWorldTreasureChestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bLidAnimating || !LidMeshComponent)
	{
		SetActorTickEnabled(false);
		bLidAnimating = false;
		return;
	}

	float TargetRoll = bLidOpening ? LidOpenedRoll : LidClosedRoll;
	float Direction = (TargetRoll > LidCurrentRoll) ? 1.0f : -1.0f;
	LidCurrentRoll += Direction * LidAnimSpeed * DeltaTime;

	// 목표 도달 확인
	bool bReached = (Direction > 0.0f)
		? (LidCurrentRoll >= TargetRoll)
		: (LidCurrentRoll <= TargetRoll);

	if (bReached)
	{
		LidCurrentRoll = TargetRoll;
		bLidAnimating = false;
		SetActorTickEnabled(false);
	}

	FRotator LidRot = LidMeshComponent->GetRelativeRotation();
	LidRot.Roll = LidCurrentRoll;
	LidMeshComponent->SetRelativeRotation(LidRot);
}

void AWjWorldTreasureChestActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	bPlayerInRange = true;
	InteractingPlayer = Pawn;

	// 입력 바인딩 (중복 방지)
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC)
	{
		EnableInput(PC);

		if (!bInputBound)
		{
			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			if (Settings && InputComponent)
			{
				UInputAction* IA = Settings->TreasureChestInteractAction.LoadSynchronous();
				if (IA)
				{
					if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
					{
						EIC->BindAction(IA, ETriggerEvent::Started, this, &AWjWorldTreasureChestActor::OnInteract);
						bInputBound = true;
					}
				}
			}
		}
	}

	// 쿨타임 만료 시 비주얼 복원 (Fix 3)
	if (!IsOnCooldown())
	{
		RemoveCooldownVisual();
		PlayLidClose();
	}

	ShowInteractionUI();

	// 쿨타임 중이면 UI 실시간 갱신 타이머 시작 (Fix 4)
	if (IsOnCooldown())
	{
		GetWorldTimerManager().SetTimer(CooldownUITimerHandle, this,
			&AWjWorldTreasureChestActor::UpdateInteractionUI, 1.0f, true);
	}

	UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: Player entered range"));
}

void AWjWorldTreasureChestActor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	// 입력 해제 + 바인딩 정리
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC)
	{
		DisableInput(PC);
	}
	if (InputComponent)
	{
		InputComponent->ClearActionBindings();
	}
	bInputBound = false;

	// 쿨타임 UI 타이머 정리
	GetWorldTimerManager().ClearTimer(CooldownUITimerHandle);

	bPlayerInRange = false;
	InteractingPlayer = nullptr;

	HideInteractionUI();

	UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: Player left range"));
}

void AWjWorldTreasureChestActor::OnInteract()
{
	if (!bPlayerInRange || !InteractingPlayer)
	{
		return;
	}

	// 로컬 쿨타임 확인 (UI용, Steam에서는 서버도 이중 검증)
	if (IsOnCooldown())
	{
		UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: On cooldown, remaining: %s"),
			*GetRemainingCooldown().ToString());
		return;
	}

	// 보상 지급 시도
	if (!TryGrantReward())
	{
		UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: Reward grant failed (Steam cooldown or error)"));
		return;
	}

	// 쿨타임 타임스탬프 저장 (UI 표시용)
	SaveCooldown();

	// 뚜껑 열기 연출
	PlayLidOpen();

	// 비주얼 전환
	ApplyCooldownVisual();

	// UI 갱신
	UpdateInteractionUI();
}

bool AWjWorldTreasureChestActor::TryGrantReward()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return false;
	}

#if WITH_STEAM
	ISteamInventory* SteamInv = SteamInventory();
	if (SteamInv)
	{
		SteamInventoryResult_t ResultHandle = k_SteamInventoryResultInvalid;
		int32 GeneratorDefId = Settings->TreasureChestGeneratorStartDefId + ChestIndex;
		SteamItemDef_t DropDef = static_cast<SteamItemDef_t>(GeneratorDefId);

		if (SteamInv->TriggerItemDrop(&ResultHandle, DropDef))
		{
			UE_LOG(LogWjWorldCurrency, Log, TEXT("TreasureChest[%d]: Steam TriggerItemDrop 요청 성공 (DefId: %d)"),
				ChestIndex, GeneratorDefId);
			SteamInv->DestroyResult(ResultHandle);

			// 2.5초 후 인벤토리 갱신 + 5초 후 재시도
			TWeakObjectPtr<AWjWorldTreasureChestActor> WeakThis(this);

			FTimerHandle FirstTimerHandle;
			GetWorldTimerManager().SetTimer(FirstTimerHandle, [WeakThis]()
			{
				if (AWjWorldTreasureChestActor* Self = WeakThis.Get())
				{
					UGameInstance* GI = Self->GetGameInstance();
					if (GI)
					{
						UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
						if (CosmeticSub)
						{
							CosmeticSub->RequestInventoryRefresh();
							UE_LOG(LogWjWorldCurrency, Log, TEXT("TreasureChest: 인벤토리 갱신 (2.5초 후)"));
						}
					}
				}
			}, 2.5f, false);

			FTimerHandle RetryTimerHandle;
			GetWorldTimerManager().SetTimer(RetryTimerHandle, [WeakThis]()
			{
				if (AWjWorldTreasureChestActor* Self = WeakThis.Get())
				{
					UGameInstance* GI = Self->GetGameInstance();
					if (GI)
					{
						UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
						if (CosmeticSub)
						{
							CosmeticSub->RequestInventoryRefresh();
							UE_LOG(LogWjWorldCurrency, Log, TEXT("TreasureChest: 인벤토리 갱신 재시도 (5초 후)"));
						}
					}
				}
			}, 5.0f, false);

			return true;
		}
		else
		{
			// TriggerItemDrop 실패 시 로컬 폴백
			UE_LOG(LogWjWorldCurrency, Warning, TEXT("TreasureChest[%d]: Steam TriggerItemDrop 실패 (DefId: %d) — 로컬 폴백"),
				ChestIndex, GeneratorDefId);

			int32 FallbackReward = Settings->TreasureChestCoinReward;
			UGameInstance* GI = GetGameInstance();
			if (GI)
			{
				UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
				if (CurrencySub)
				{
					CurrencySub->GrantCurrencyLocally(ECurrencyType::Coin, FallbackReward);
					UE_LOG(LogWjWorldCurrency, Log, TEXT("TreasureChest: 로컬 폴백 보상 %d Coin 지급"), FallbackReward);
				}
			}
			return true; // 폴백 지급 성공으로 처리
		}
	}
#endif

	// 비Steam 폴백: 로컬 재화 부여
	int32 RewardAmount = Settings->TreasureChestCoinReward;
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
		if (CurrencySub)
		{
			CurrencySub->GrantCurrencyLocally(ECurrencyType::Coin, RewardAmount);
			UE_LOG(LogWjWorldCurrency, Log, TEXT("TreasureChest: 로컬 보상 %d Coin 지급"), RewardAmount);
		}
	}

	return true;
}

void AWjWorldTreasureChestActor::ShowInteractionUI()
{
	if (InteractionWidgetComponent)
	{
		InteractionWidgetComponent->SetVisibility(true);
		UpdateInteractionUI();
	}
}

void AWjWorldTreasureChestActor::HideInteractionUI()
{
	if (InteractionWidgetComponent)
	{
		InteractionWidgetComponent->SetVisibility(false);
	}
}

void AWjWorldTreasureChestActor::UpdateInteractionUI()
{
	if (!InteractionWidget)
	{
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	int32 RewardAmount = Settings ? Settings->TreasureChestCoinReward : 50;

	if (IsOnCooldown())
	{
		FTimespan Remaining = GetRemainingCooldown();
		int32 Hours = static_cast<int32>(Remaining.GetTotalHours());
		int32 Minutes = Remaining.GetMinutes();
		int32 Seconds = Remaining.GetSeconds();

		FString CooldownText = FString::Printf(TEXT("Next open: %02d:%02d:%02d"), Hours, Minutes, Seconds);
		InteractionWidget->SetInteractionText(CooldownText);
	}
	else
	{
		// 쿨타임 만료 감지 → 타이머 정지 + 비주얼 복원
		GetWorldTimerManager().ClearTimer(CooldownUITimerHandle);
		RemoveCooldownVisual();
		PlayLidClose();

		FString InteractText = FString::Printf(TEXT("Open Chest (%d Coin)"), RewardAmount);
		InteractionWidget->SetInteractionText(InteractText);
	}
}

// === Cooldown ===

FString AWjWorldTreasureChestActor::GetChestKey() const
{
	FVector Loc = GetActorLocation();
	return FString::Printf(TEXT("Chest_%d_%d_%d"),
		FMath::RoundToInt(Loc.X),
		FMath::RoundToInt(Loc.Y),
		FMath::RoundToInt(Loc.Z));
}

FString AWjWorldTreasureChestActor::GetConfigPath() const
{
	return GGameUserSettingsIni;
}

bool AWjWorldTreasureChestActor::IsOnCooldown() const
{
	if (CachedLastOpenedTime.GetTicks() == 0)
	{
		return false;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	float CooldownSeconds = Settings ? Settings->TreasureChestCooldownSeconds : 86400.0f;
	return (FDateTime::UtcNow() - CachedLastOpenedTime).GetTotalSeconds() < CooldownSeconds;
}

FTimespan AWjWorldTreasureChestActor::GetRemainingCooldown() const
{
	if (CachedLastOpenedTime.GetTicks() == 0)
	{
		return FTimespan::Zero();
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	float CooldownSeconds = Settings ? Settings->TreasureChestCooldownSeconds : 86400.0f;

	FDateTime CooldownEnd = CachedLastOpenedTime + FTimespan::FromSeconds(CooldownSeconds);
	FTimespan Remaining = CooldownEnd - FDateTime::UtcNow();

	if (Remaining.GetTotalSeconds() > 0)
	{
		return Remaining;
	}
	return FTimespan::Zero();
}

void AWjWorldTreasureChestActor::SaveCooldown()
{
	CachedLastOpenedTime = FDateTime::UtcNow();

	FString Key = GetChestKey();
	FString ConfigPath = GetConfigPath();

	GConfig->SetString(*ConfigSection, *Key, *CachedLastOpenedTime.ToIso8601(), ConfigPath);
	GConfig->Flush(false, ConfigPath);

	UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: Cooldown saved for %s"), *Key);
}

void AWjWorldTreasureChestActor::ResetCooldown()
{
	CachedLastOpenedTime = FDateTime();

	// GConfig에서도 제거
	FString Key = GetChestKey();
	FString ConfigPath = GetConfigPath();
	GConfig->RemoveKey(*ConfigSection, *Key, ConfigPath);
	GConfig->Flush(false, ConfigPath);

	// 비주얼 복원
	RemoveCooldownVisual();
	PlayLidClose();

	UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: Cooldown reset for %s"), *Key);
}

// === Lid Animation ===

void AWjWorldTreasureChestActor::PlayLidOpen()
{
	bLidOpening = true;
	bLidAnimating = true;
	SetActorTickEnabled(true);
}

void AWjWorldTreasureChestActor::PlayLidClose()
{
	bLidOpening = false;
	bLidAnimating = true;
	SetActorTickEnabled(true);
}

void AWjWorldTreasureChestActor::SetLidOpenImmediate()
{
	LidCurrentRoll = LidOpenedRoll;
	bLidAnimating = false;

	if (LidMeshComponent)
	{
		FRotator LidRot = LidMeshComponent->GetRelativeRotation();
		LidRot.Roll = LidOpenedRoll;
		LidMeshComponent->SetRelativeRotation(LidRot);
	}
}

void AWjWorldTreasureChestActor::SetLidCloseImmediate()
{
	LidCurrentRoll = LidClosedRoll;
	bLidAnimating = false;

	if (LidMeshComponent)
	{
		FRotator LidRot = LidMeshComponent->GetRelativeRotation();
		LidRot.Roll = LidClosedRoll;
		LidMeshComponent->SetRelativeRotation(LidRot);
	}
}

// === Visual ===

void AWjWorldTreasureChestActor::ApplyCooldownVisual()
{
	if (!MeshComponent)
	{
		return;
	}

	int32 NumMats = MeshComponent->GetNumMaterials();
	for (int32 i = 0; i < NumMats; ++i)
	{
		UMaterialInstanceDynamic* DMI = MeshComponent->CreateAndSetMaterialInstanceDynamic(i);
		if (DMI)
		{
			// 어두운 회색 틴트
			DMI->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
			DMI->SetScalarParameterValue(TEXT("Opacity"), 0.5f);
		}
	}
}

void AWjWorldTreasureChestActor::RemoveCooldownVisual()
{
	// PlacedObjectActor의 원본 머티리얼 복원 패턴과 동일
	// 메시 재로드로 원본 머티리얼 복원
	if (MeshComponent && MeshComponent->GetStaticMesh())
	{
		UStaticMesh* CurrentMesh = MeshComponent->GetStaticMesh();
		int32 NumMats = CurrentMesh->GetStaticMaterials().Num();
		for (int32 i = 0; i < NumMats; ++i)
		{
			MeshComponent->SetMaterial(i, CurrentMesh->GetMaterial(i));
		}
	}
}
