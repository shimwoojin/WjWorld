// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "Engine/StreamableManager.h"
#include "WjWorldCosmeticComponent.generated.h"

class UWjWorldCosmeticCatalogDataAsset;
struct FCosmeticItemDefinition;

/**
 * 캐릭터에 부착되어 코스메틱 비주얼을 적용하는 컴포넌트
 * - 로드아웃 기반 비주얼 적용
 * - 비동기 에셋 로드 (FStreamableManager)
 * - 기본 스킨 백업/복원
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WJWORLD_API UWjWorldCosmeticComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWjWorldCosmeticComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 카탈로그 설정 */
	void SetCatalog(UWjWorldCosmeticCatalogDataAsset* InCatalog);

	/** 전체 로드아웃 적용 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ApplyLoadout(const FCosmeticLoadout& InLoadout);

	/** 개별 슬롯에 아이템 적용 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ApplySlot(ECosmeticSlot Slot, FName ItemId);

	/** 개별 슬롯 제거 (기본 스킨으로 복원) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ClearSlot(ECosmeticSlot Slot);

	/** 모든 코스메틱 제거 (기본 스킨 복원) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ClearAll();

	/** 현재 적용된 로드아웃 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	const FCosmeticLoadout& GetAppliedLoadout() const { return AppliedLoadout; }

protected:
	/** CosmeticSubsystem의 OnLoadoutChanged 핸들러 */
	UFUNCTION()
	void OnLoadoutChangedHandler(ECosmeticSlot Slot, FName ItemId);

private:
	/** 비동기 에셋 로드 후 메시 적용 */
	void OnAssetLoaded(ECosmeticSlot Slot, FName ItemId);

	/** 슬롯에 해당하는 메시 컴포넌트 반환 또는 생성 */
	UMeshComponent* GetOrCreateSlotMeshComponent(ECosmeticSlot Slot);

	/** 기본 메시 백업 */
	void BackupDefaultMeshes();

	/** 기본 메시로 복원 */
	void RestoreDefaultMesh(ECosmeticSlot Slot);

	UPROPERTY()
	TObjectPtr<UWjWorldCosmeticCatalogDataAsset> Catalog;

	/** 현재 적용된 로드아웃 */
	FCosmeticLoadout AppliedLoadout;

	/** 슬롯별 동적 생성된 메시 컴포넌트 */
	UPROPERTY()
	TMap<ECosmeticSlot, TObjectPtr<UMeshComponent>> SlotMeshComponents;

	/** 기본 메시 백업 (Head/Body 슬롯용) */
	UPROPERTY()
	TMap<ECosmeticSlot, TSoftObjectPtr<USkeletalMesh>> DefaultMeshes;

	/** 비동기 로드 핸들 */
	FStreamableManager StreamableManager;
	TMap<ECosmeticSlot, TSharedPtr<FStreamableHandle>> ActiveStreamHandles;

	/** 기본 메시 백업 완료 여부 */
	bool bDefaultMeshesBackedUp = false;

	/** 슬롯별 기본 소켓 이름 반환 */
	static FName GetDefaultSocketName(ECosmeticSlot Slot);
};
