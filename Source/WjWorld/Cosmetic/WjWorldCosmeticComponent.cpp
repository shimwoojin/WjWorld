// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldCosmeticComponent.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "WjWorldLogCategories.h"

UWjWorldCosmeticComponent::UWjWorldCosmeticComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UWjWorldCosmeticComponent::BeginPlay()
{
	Super::BeginPlay();

	BackupDefaultMeshes();

	// CosmeticSubsystem 구독 및 초기화
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			// 델리게이트 구독
			CosmeticSub->OnLoadoutChanged.AddDynamic(this, &ThisClass::OnLoadoutChangedHandler);

			// 카탈로그 설정
			SetCatalog(CosmeticSub->GetCatalog());

			// 초기 로드아웃 적용
			ApplyLoadout(CosmeticSub->GetLoadout());

			UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticComponent: 서브시스템 구독 완료"));
		}
	}
}

void UWjWorldCosmeticComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 비동기 로드 핸들 정리
	for (auto& Pair : ActiveStreamHandles)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->CancelHandle();
		}
	}
	ActiveStreamHandles.Reset();

	// CosmeticSubsystem 구독 해제
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			CosmeticSub->OnLoadoutChanged.RemoveDynamic(this, &ThisClass::OnLoadoutChangedHandler);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UWjWorldCosmeticComponent::SetCatalog(UWjWorldCosmeticCatalogDataAsset* InCatalog)
{
	Catalog = InCatalog;
}

void UWjWorldCosmeticComponent::ApplyLoadout(const FCosmeticLoadout& InLoadout)
{
	// 기존 코스메틱 제거
	ClearAll();

	// 새 로드아웃 적용
	for (const FCosmeticSlotEntry& Entry : InLoadout.Entries)
	{
		ApplySlot(Entry.Slot, Entry.ItemId);
	}
}

void UWjWorldCosmeticComponent::ApplySlot(ECosmeticSlot Slot, FName ItemId)
{
	if (Slot == ECosmeticSlot::None || ItemId.IsNone())
	{
		return;
	}

	if (!Catalog)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("카탈로그가 설정되지 않았습니다."));
		return;
	}

	const FCosmeticItemDefinition* Def = Catalog->FindByItemId(ItemId);
	if (!Def)
	{
		UE_LOG(LogWjWorldCosmetic, Warning, TEXT("카탈로그에서 아이템 '%s'을 찾을 수 없습니다."), *ItemId.ToString());
		return;
	}

	// 기존 핸들 취소
	if (TSharedPtr<FStreamableHandle>* ExistingHandle = ActiveStreamHandles.Find(Slot))
	{
		if (ExistingHandle->IsValid())
		{
			(*ExistingHandle)->CancelHandle();
		}
	}

	AppliedLoadout.Equip(Slot, ItemId);

	// 비동기 에셋 로드
	TArray<FSoftObjectPath> AssetsToLoad;

	if (!Def->SkeletalMesh.IsNull())
	{
		AssetsToLoad.Add(Def->SkeletalMesh.ToSoftObjectPath());
	}
	if (!Def->StaticMesh.IsNull())
	{
		AssetsToLoad.Add(Def->StaticMesh.ToSoftObjectPath());
	}

	if (AssetsToLoad.Num() > 0)
	{
		TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
			AssetsToLoad,
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnAssetLoaded, Slot, ItemId)
		);
		ActiveStreamHandles.Add(Slot, Handle);

		UE_LOG(LogWjWorldCosmetic, Log, TEXT("비동기 로드 시작: 슬롯 %d ← %s (%d 에셋)"),
			static_cast<int32>(Slot), *ItemId.ToString(), AssetsToLoad.Num());
	}
}

void UWjWorldCosmeticComponent::ClearSlot(ECosmeticSlot Slot)
{
	// 비동기 핸들 취소
	if (TSharedPtr<FStreamableHandle>* Handle = ActiveStreamHandles.Find(Slot))
	{
		if (Handle->IsValid())
		{
			(*Handle)->CancelHandle();
		}
		ActiveStreamHandles.Remove(Slot);
	}

	AppliedLoadout.Unequip(Slot);
	RestoreDefaultMesh(Slot);

	// 동적 메시 컴포넌트 제거
	if (TObjectPtr<UMeshComponent>* MeshComp = SlotMeshComponents.Find(Slot))
	{
		if (*MeshComp)
		{
			(*MeshComp)->DestroyComponent();
		}
		SlotMeshComponents.Remove(Slot);
	}
}

void UWjWorldCosmeticComponent::ClearAll()
{
	TArray<ECosmeticSlot> Slots = AppliedLoadout.GetEquippedSlots();

	for (ECosmeticSlot Slot : Slots)
	{
		ClearSlot(Slot);
	}

	AppliedLoadout.Reset();
}

void UWjWorldCosmeticComponent::OnAssetLoaded(ECosmeticSlot Slot, FName ItemId)
{
	if (!IsValid(this) || !GetOwner())
	{
		return;
	}

	// 로드 완료 시 해당 슬롯이 여전히 같은 아이템인지 확인
	FName CurrentItem = AppliedLoadout.GetEquippedItem(Slot);
	if (CurrentItem != ItemId)
	{
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("비동기 로드 완료되었으나 슬롯이 변경됨 (기대: %s, 현재: %s)"),
			*ItemId.ToString(), *CurrentItem.ToString());
		return;
	}

	if (!Catalog)
	{
		return;
	}

	const FCosmeticItemDefinition* Def = Catalog->FindByItemId(ItemId);
	if (!Def)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	// 소켓 이름 결정 (아이템 정의 우선, 없으면 슬롯 기본값)
	FName SocketName = Def->AttachSocketName.IsNone() ? GetDefaultSocketName(Slot) : Def->AttachSocketName;

	// SkeletalMesh 적용 (Head, Body 슬롯)
	if (!Def->SkeletalMesh.IsNull() && Def->SkeletalMesh.IsValid())
	{
		USkeletalMesh* LoadedMesh = Def->SkeletalMesh.Get();
		if (LoadedMesh)
		{
			// Body 슬롯: 캐릭터의 메인 메시 교체
			if (Slot == ECosmeticSlot::Body)
			{
				USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
				if (MeshComp)
				{
					MeshComp->SetSkeletalMesh(LoadedMesh);
					UE_LOG(LogWjWorldCosmetic, Log, TEXT("Body 메시 적용: %s"), *LoadedMesh->GetName());
				}
			}
			else
			{
				// 다른 슬롯: 소켓에 부착
				USkeletalMeshComponent* SlotMesh = NewObject<USkeletalMeshComponent>(OwnerCharacter);
				SlotMesh->SetSkeletalMesh(LoadedMesh);
				SlotMesh->AttachToComponent(
					OwnerCharacter->GetMesh(),
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					SocketName
				);

				// 오프셋 적용
				SlotMesh->SetRelativeLocation(Def->AttachLocationOffset);
				SlotMesh->SetRelativeRotation(Def->AttachRotationOffset);
				SlotMesh->SetRelativeScale3D(Def->AttachScale);

				SlotMesh->RegisterComponent();
				SlotMeshComponents.Add(Slot, SlotMesh);

				UE_LOG(LogWjWorldCosmetic, Log, TEXT("슬롯 %d 메시 소켓 부착: %s → %s"),
					static_cast<int32>(Slot), *LoadedMesh->GetName(), *SocketName.ToString());
			}
		}
	}

	// StaticMesh 적용 (Back, Effect 슬롯)
	if (!Def->StaticMesh.IsNull() && Def->StaticMesh.IsValid())
	{
		UStaticMesh* LoadedStaticMesh = Def->StaticMesh.Get();
		if (LoadedStaticMesh)
		{
			UStaticMeshComponent* SlotStaticMesh = NewObject<UStaticMeshComponent>(OwnerCharacter);
			SlotStaticMesh->SetStaticMesh(LoadedStaticMesh);
			SlotStaticMesh->AttachToComponent(
				OwnerCharacter->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				SocketName
			);

			// 오프셋 적용
			SlotStaticMesh->SetRelativeLocation(Def->AttachLocationOffset);
			SlotStaticMesh->SetRelativeRotation(Def->AttachRotationOffset);
			SlotStaticMesh->SetRelativeScale3D(Def->AttachScale);

			SlotStaticMesh->RegisterComponent();
			SlotMeshComponents.Add(Slot, SlotStaticMesh);

			UE_LOG(LogWjWorldCosmetic, Log, TEXT("슬롯 %d 스태틱 메시 소켓 부착: %s → %s"),
				static_cast<int32>(Slot), *LoadedStaticMesh->GetName(), *SocketName.ToString());
		}
	}

	// 로드 핸들 정리
	ActiveStreamHandles.Remove(Slot);
}

UMeshComponent* UWjWorldCosmeticComponent::GetOrCreateSlotMeshComponent(ECosmeticSlot Slot)
{
	if (TObjectPtr<UMeshComponent>* Found = SlotMeshComponents.Find(Slot))
	{
		return *Found;
	}
	return nullptr;
}

void UWjWorldCosmeticComponent::BackupDefaultMeshes()
{
	if (bDefaultMeshesBackedUp)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (MeshComp && MeshComp->GetSkeletalMeshAsset())
	{
		DefaultMeshes.Add(ECosmeticSlot::Body, MeshComp->GetSkeletalMeshAsset());
	}

	bDefaultMeshesBackedUp = true;
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("기본 메시 백업 완료"));
}

void UWjWorldCosmeticComponent::RestoreDefaultMesh(ECosmeticSlot Slot)
{
	if (Slot != ECosmeticSlot::Body)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (const TSoftObjectPtr<USkeletalMesh>* DefaultMesh = DefaultMeshes.Find(Slot))
	{
		if (DefaultMesh->IsValid())
		{
			USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
			if (MeshComp)
			{
				MeshComp->SetSkeletalMesh(DefaultMesh->Get());
				UE_LOG(LogWjWorldCosmetic, Log, TEXT("기본 메시 복원: 슬롯 %d"), static_cast<int32>(Slot));
			}
		}
	}
}

void UWjWorldCosmeticComponent::OnLoadoutChangedHandler(ECosmeticSlot Slot, FName ItemId)
{
	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticComponent: 로드아웃 변경 감지 - 슬롯 %d, 아이템 %s"),
		static_cast<int32>(Slot), *ItemId.ToString());

	if (ItemId.IsNone())
	{
		// 아이템 해제
		ClearSlot(Slot);
	}
	else
	{
		// 아이템 장착
		ApplySlot(Slot, ItemId);
	}
}

FName UWjWorldCosmeticComponent::GetDefaultSocketName(ECosmeticSlot Slot)
{
	switch (Slot)
	{
	case ECosmeticSlot::Head:
		return FName(TEXT("head"));  // 스켈레톤의 head 본/소켓
	case ECosmeticSlot::Body:
		return NAME_None;  // Body는 메시 교체 방식
	case ECosmeticSlot::Back:
		return FName(TEXT("spine_03"));  // 등 부착용
	case ECosmeticSlot::Effect:
		return FName(TEXT("root"));  // 이펙트는 루트
	default:
		return NAME_None;
	}
}
