// Fill out your copyright notice in the Description page of Project Settings.

#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "WjWorldLogCategories.h"

const FCosmeticItemDefinition* UWjWorldCosmeticCatalogDataAsset::FindByItemId(FName ItemId) const
{
	BuildLookupMaps();

	if (const int32* Index = ItemIdToIndex.Find(ItemId))
	{
		return &Items[*Index];
	}
	return nullptr;
}

const FCosmeticItemDefinition* UWjWorldCosmeticCatalogDataAsset::FindBySteamItemDefId(int32 SteamItemDefId) const
{
	BuildLookupMaps();

	if (const int32* Index = SteamDefIdToIndex.Find(SteamItemDefId))
	{
		return &Items[*Index];
	}
	return nullptr;
}

FName UWjWorldCosmeticCatalogDataAsset::SteamItemDefIdToItemId(int32 SteamItemDefId) const
{
	if (const FCosmeticItemDefinition* Def = FindBySteamItemDefId(SteamItemDefId))
	{
		return Def->ItemId;
	}
	return NAME_None;
}

int32 UWjWorldCosmeticCatalogDataAsset::ItemIdToSteamItemDefId(FName ItemId) const
{
	if (const FCosmeticItemDefinition* Def = FindByItemId(ItemId))
	{
		return Def->SteamItemDefId;
	}
	return 0;
}

TArray<const FCosmeticItemDefinition*> UWjWorldCosmeticCatalogDataAsset::GetItemsBySlot(ECosmeticSlot Slot) const
{
	TArray<const FCosmeticItemDefinition*> Result;
	for (const FCosmeticItemDefinition& Item : Items)
	{
		if (Item.Slot == Slot)
		{
			Result.Add(&Item);
		}
	}
	return Result;
}

//#if WITH_EDITOR
//void UWjWorldCosmeticCatalogDataAsset::PreSave(FObjectPreSaveContext SaveContext)
//{
//	Super::PreSave(SaveContext);
//
//	// 에디터에서 저장 시 중복 ItemId 검증
//	TSet<FName> SeenItemIds;
//	TSet<int32> SeenSteamIds;
//
//	for (int32 i = 0; i < Items.Num(); ++i)
//	{
//		const FCosmeticItemDefinition& Item = Items[i];
//
//		if (Item.ItemId.IsNone())
//		{
//			UE_LOG(LogWjWorldCosmetic, Warning, TEXT("CosmeticCatalog [%s]: 인덱스 %d에 빈 ItemId가 있습니다."),
//				*GetName(), i);
//			continue;
//		}
//
//		if (SeenItemIds.Contains(Item.ItemId))
//		{
//			UE_LOG(LogWjWorldCosmetic, Error, TEXT("CosmeticCatalog [%s]: 중복 ItemId '%s' (인덱스 %d)"),
//				*GetName(), *Item.ItemId.ToString(), i);
//		}
//		SeenItemIds.Add(Item.ItemId);
//
//		if (Item.SteamItemDefId != 0)
//		{
//			if (SeenSteamIds.Contains(Item.SteamItemDefId))
//			{
//				UE_LOG(LogWjWorldCosmetic, Error, TEXT("CosmeticCatalog [%s]: 중복 SteamItemDefId %d (인덱스 %d)"),
//					*GetName(), Item.SteamItemDefId, i);
//			}
//			SeenSteamIds.Add(Item.SteamItemDefId);
//		}
//	}
//
//	// 저장 시 lookup 무효화
//	bLookupDirty = true;
//}
//#endif

void UWjWorldCosmeticCatalogDataAsset::BuildLookupMaps() const
{
	if (!bLookupDirty)
	{
		return;
	}

	ItemIdToIndex.Reset();
	SteamDefIdToIndex.Reset();

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		const FCosmeticItemDefinition& Item = Items[i];
		if (!Item.ItemId.IsNone())
		{
			ItemIdToIndex.Add(Item.ItemId, i);
		}
		if (Item.SteamItemDefId != 0)
		{
			SteamDefIdToIndex.Add(Item.SteamItemDefId, i);
		}
	}

	bLookupDirty = false;
}
