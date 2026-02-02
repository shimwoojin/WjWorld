// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Profile/CharacterPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "WjWorldLogCategories.h"

ACharacterPreviewActor::ACharacterPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// SkeletalMesh (캐릭터 기본 메시)
	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMeshComponent->SetupAttachment(SceneRoot);
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 키 라이트
	KeyLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(SceneRoot);
	KeyLight->SetRelativeLocation(FVector(200.f, 100.f, 200.f));
	KeyLight->SetRelativeRotation(FRotator(-30.f, -150.f, 0.f));
	KeyLight->SetIntensity(5000.f);
	KeyLight->SetAttenuationRadius(1000.f);
	KeyLight->SetOuterConeAngle(60.f);
	KeyLight->SetInnerConeAngle(40.f);

	// Scene Capture
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCaptureComponent->SetupAttachment(SceneRoot);
	SceneCaptureComponent->SetRelativeLocation(FVector(200.f, 0.f, 90.f));
	SceneCaptureComponent->SetRelativeRotation(FRotator(-10.f, 180.f, 0.f));
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCaptureComponent->CaptureSource = SCS_FinalColorLDR;
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
}

void ACharacterPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	// RenderTarget 동적 생성
	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->InitAutoFormat(RenderTargetWidth, RenderTargetHeight);
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->UpdateResourceImmediate();

	SceneCaptureComponent->TextureTarget = RenderTarget;

	// ShowOnlyList에 프리뷰 메시 추가
	SceneCaptureComponent->ShowOnlyComponents.Add(PreviewMeshComponent);
}

void ACharacterPreviewActor::SetupPreview(const FCosmeticLoadout& Loadout)
{
	// 기존 코스메틱 메시 제거
	for (auto& Pair : SlotMeshComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	SlotMeshComponents.Empty();
	ActiveStreamHandles.Empty();

	// 카탈로그 가져오기
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		return;
	}

	UWjWorldCosmeticCatalogDataAsset* Catalog = CosmeticSub->GetCatalog();
	if (!Catalog)
	{
		RefreshCapture();
		return;
	}

	// 코스메틱 로드아웃 적용
	for (const FCosmeticSlotEntry& Entry : Loadout.Entries)
	{
		if (Entry.Slot == ECosmeticSlot::None || Entry.ItemId.IsNone())
		{
			continue;
		}

		const FCosmeticItemDefinition* ItemDef = Catalog->FindByItemId(Entry.ItemId);
		if (!ItemDef)
		{
			continue;
		}

		// StaticMesh 또는 SkeletalMesh 비동기 로드
		FSoftObjectPath MeshPath;
		if (!ItemDef->StaticMesh.IsNull())
		{
			MeshPath = ItemDef->StaticMesh.ToSoftObjectPath();
		}
		else if (!ItemDef->SkeletalMesh.IsNull())
		{
			MeshPath = ItemDef->SkeletalMesh.ToSoftObjectPath();
		}

		if (!MeshPath.IsNull())
		{
			ECosmeticSlot CapturedSlot = Entry.Slot;
			FName CapturedItemId = Entry.ItemId;

			TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
				MeshPath,
				FStreamableDelegate::CreateWeakLambda(this, [this, CapturedSlot, CapturedItemId]()
				{
					OnCosmeticAssetLoaded(CapturedSlot, CapturedItemId);
				})
			);

			if (Handle.IsValid())
			{
				ActiveStreamHandles.Add(CapturedSlot, Handle);
			}
		}
	}

	// 코스메틱이 없으면 바로 캡처
	if (Loadout.IsEmpty())
	{
		RefreshCapture();
	}
}

void ACharacterPreviewActor::OnCosmeticAssetLoaded(ECosmeticSlot Slot, FName ItemId)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub || !CosmeticSub->GetCatalog())
	{
		return;
	}

	const FCosmeticItemDefinition* ItemDef = CosmeticSub->GetCatalog()->FindByItemId(ItemId);
	if (!ItemDef)
	{
		return;
	}

	// StaticMesh 우선, 없으면 SkeletalMesh 시도
	UObject* LoadedAsset = nullptr;
	if (!ItemDef->StaticMesh.IsNull())
	{
		LoadedAsset = ItemDef->StaticMesh.ToSoftObjectPath().ResolveObject();
	}
	else if (!ItemDef->SkeletalMesh.IsNull())
	{
		LoadedAsset = ItemDef->SkeletalMesh.ToSoftObjectPath().ResolveObject();
	}

	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(LoadedAsset))
	{
		UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
		MeshComp->SetupAttachment(PreviewMeshComponent);
		MeshComp->SetStaticMesh(StaticMesh);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->RegisterComponent();

		SlotMeshComponents.Add(Slot, MeshComp);
		SceneCaptureComponent->ShowOnlyComponents.Add(MeshComp);
	}

	// 모든 로드가 완료되었는지 확인
	ActiveStreamHandles.Remove(Slot);
	if (ActiveStreamHandles.Num() == 0)
	{
		RefreshCapture();
	}
}

void ACharacterPreviewActor::RefreshCapture()
{
	if (SceneCaptureComponent && RenderTarget)
	{
		SceneCaptureComponent->CaptureScene();

		UE_LOG(LogWjWorld, Log, TEXT("CharacterPreviewActor: Scene captured"));
	}
}
