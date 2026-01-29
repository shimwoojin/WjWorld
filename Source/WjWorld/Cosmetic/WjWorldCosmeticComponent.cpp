// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldCosmeticComponent.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
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
	for (const auto& Pair : InLoadout.EquippedItems)
	{
		ApplySlot(Pair.Key, Pair.Value);
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
	TArray<ECosmeticSlot> Slots;
	AppliedLoadout.EquippedItems.GetKeys(Slots);

	for (ECosmeticSlot Slot : Slots)
	{
		ClearSlot(Slot);
	}

	AppliedLoadout.EquippedItems.Reset();
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
				// 다른 슬롯: 부착 메시 컴포넌트 생성
				USkeletalMeshComponent* SlotMesh = NewObject<USkeletalMeshComponent>(OwnerCharacter);
				SlotMesh->SetSkeletalMesh(LoadedMesh);
				SlotMesh->AttachToComponent(
					OwnerCharacter->GetMesh(),
					FAttachmentTransformRules::SnapToTargetNotIncludingScale
				);
				SlotMesh->RegisterComponent();
				SlotMeshComponents.Add(Slot, SlotMesh);

				UE_LOG(LogWjWorldCosmetic, Log, TEXT("슬롯 %d 메시 부착: %s"),
					static_cast<int32>(Slot), *LoadedMesh->GetName());
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
				FAttachmentTransformRules::SnapToTargetNotIncludingScale
			);
			SlotStaticMesh->RegisterComponent();
			SlotMeshComponents.Add(Slot, SlotStaticMesh);

			UE_LOG(LogWjWorldCosmetic, Log, TEXT("슬롯 %d 스태틱 메시 부착: %s"),
				static_cast<int32>(Slot), *LoadedStaticMesh->GetName());
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
