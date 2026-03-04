// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "Engine/StreamableManager.h"
#include "CharacterPreviewActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USpotLightComponent;
class UWjWorldCosmeticCatalogDataAsset;
class UMeshComponent;

/**
 * 3D 캐릭터 프리뷰 액터
 * - 월드에서 보이지 않는 위치에 스폰
 * - SkeletalMesh + 코스메틱 메시 표시
 * - SceneCaptureComponent2D로 RenderTarget에 캡처
 * - 위젯에서 RenderTarget을 UImage로 표시
 */
UCLASS()
class WJWORLD_API ACharacterPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ACharacterPreviewActor();

	/** Pawn에서 SkeletalMesh와 AnimBlueprint 복사 */
	void SetupFromPawn(APawn* SourcePawn);

	/** 코스메틱 로드아웃으로 프리뷰 설정 */
	void SetupPreview(const FCosmeticLoadout& Loadout);

	/** 렌더 타겟 반환 */
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	/** alpha 반전 처리된 프리뷰 머티리얼 반환 (Image 브러시용) */
	UMaterialInstanceDynamic* GetPreviewMaterial();

	/** 기본 메시가 설정되어 있는지 확인 */
	bool HasBaseMesh() const;

	/** 캡처 갱신 */
	void RefreshCapture();

protected:
	virtual void BeginPlay() override;

private:
	/** 코스메틱 에셋 비동기 로드 완료 후 적용 */
	void OnCosmeticAssetLoaded(ECosmeticSlot Slot, FName ItemId);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** alpha 반전 프리뷰 머티리얼 (M_CharacterPreview) */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> PreviewMaterialBase;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMID;

	/** 슬롯별 코스메틱 메시 컴포넌트 (StaticMesh 또는 SkeletalMesh) */
	UPROPERTY()
	TMap<ECosmeticSlot, TObjectPtr<UMeshComponent>> SlotMeshComponents;

	/** 원본 메시/애니메이션 백업 (Body 프리뷰 교체 후 복원용) */
	TSoftObjectPtr<USkeletalMesh> OriginalMesh;
	TSubclassOf<UAnimInstance> OriginalAnimClass;

	/** 슬롯별 기본 부착 소켓 이름 반환 */
	static FName GetDefaultSocketName(ECosmeticSlot Slot);

	/** 비동기 로드 핸들 */
	FStreamableManager StreamableManager;
	TMap<ECosmeticSlot, TSharedPtr<FStreamableHandle>> ActiveStreamHandles;

	/** 렌더 타겟 크기 */
	static constexpr int32 RenderTargetWidth = 500;
	static constexpr int32 RenderTargetHeight = 1000;
};
