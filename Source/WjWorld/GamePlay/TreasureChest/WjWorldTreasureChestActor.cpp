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
#include "WjWorldLogCategories.h"

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
	// 입력 해제
	if (InteractingPlayer)
	{
		APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController());
		if (PC)
		{
			DisableInput(PC);
		}
	}

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

	// 입력 바인딩
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC)
	{
		EnableInput(PC);

		const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
		if (Settings && InputComponent)
		{
			UInputAction* IA = Settings->TreasureChestInteractAction.LoadSynchronous();
			if (IA)
			{
				if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
				{
					EIC->BindAction(IA, ETriggerEvent::Started, this, &AWjWorldTreasureChestActor::OnInteract);
				}
			}
		}
	}

	ShowInteractionUI();

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

	// 입력 해제
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC)
	{
		DisableInput(PC);
	}

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

	// 쿨타임 확인
	if (IsOnCooldown())
	{
		UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: On cooldown, remaining: %s"),
			*GetRemainingCooldown().ToString());
		return;
	}

	// Coin 보상 지급
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	int32 RewardAmount = Settings ? Settings->TreasureChestCoinReward : 50;

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
		if (CurrencySub)
		{
			CurrencySub->GrantCurrencyLocally(ECurrencyType::Coin, RewardAmount);
			UE_LOG(LogWjWorldCurrency, Log, TEXT("TreasureChest: Granted %d Coin"), RewardAmount);
		}
	}

	// 쿨타임 저장
	SaveCooldown();

	// 뚜껑 열기 연출
	PlayLidOpen();

	// 비주얼 전환
	ApplyCooldownVisual();

	// UI 갱신
	UpdateInteractionUI();
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
	return FPaths::GeneratedConfigDir() + TEXT("TreasureChestCooldown.ini");
}

bool AWjWorldTreasureChestActor::IsOnCooldown() const
{
	FString Key = GetChestKey();
	FString LastOpenedStr;
	FString ConfigPath = GetConfigPath();

	if (GConfig->GetString(*ConfigSection, *Key, LastOpenedStr, ConfigPath))
	{
		FDateTime LastOpened;
		if (FDateTime::Parse(LastOpenedStr, LastOpened))
		{
			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			float CooldownSeconds = Settings ? Settings->TreasureChestCooldownSeconds : 86400.0f;
			return (FDateTime::UtcNow() - LastOpened).GetTotalSeconds() < CooldownSeconds;
		}
	}
	return false;
}

FTimespan AWjWorldTreasureChestActor::GetRemainingCooldown() const
{
	FString Key = GetChestKey();
	FString LastOpenedStr;
	FString ConfigPath = GetConfigPath();

	if (GConfig->GetString(*ConfigSection, *Key, LastOpenedStr, ConfigPath))
	{
		FDateTime LastOpened;
		if (FDateTime::Parse(LastOpenedStr, LastOpened))
		{
			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			float CooldownSeconds = Settings ? Settings->TreasureChestCooldownSeconds : 86400.0f;

			FDateTime CooldownEnd = LastOpened + FTimespan::FromSeconds(CooldownSeconds);
			FTimespan Remaining = CooldownEnd - FDateTime::UtcNow();

			if (Remaining.GetTotalSeconds() > 0)
			{
				return Remaining;
			}
		}
	}
	return FTimespan::Zero();
}

void AWjWorldTreasureChestActor::SaveCooldown()
{
	FString Key = GetChestKey();
	FString ConfigPath = GetConfigPath();

	GConfig->SetString(*ConfigSection, *Key, *FDateTime::UtcNow().ToIso8601(), ConfigPath);
	GConfig->Flush(false, ConfigPath);

	UE_LOG(LogWjWorld, Log, TEXT("TreasureChest: Cooldown saved for %s"), *Key);
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
