// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "WjWorldPlacementComponent.generated.h"

class AWjWorldPlacementPreviewActor;
class AWjWorldPlacedObjectActor;
class AWjWorldGameStateLobby;
class UWjWorldPlaceableObjectDataAsset;
class UInputMappingContext;
class UInputAction;
struct FPlaceableObjectDefinition;
struct FInputActionValue;

/**
 * 배치 모드 상태
 */
UENUM(BlueprintType)
enum class EPlacementMode : uint8
{
	None,
	Placing,
	Deleting
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementModeChanged, EPlacementMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectPlaced);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectDeleted);

/**
 * 로비 오브젝트 배치 핵심 컴포넌트
 * PlayerControllerLobby에 부착되어 배치/삭제 로직을 처리
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WJWORLD_API UWjWorldPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWjWorldPlacementComponent();

	//~ 모드 전환
	void EnterPlacementMode();
	void ExitPlacementMode();
	void ToggleDeleteMode();
	EPlacementMode GetCurrentMode() const { return CurrentMode; }

	//~ 오브젝트 조작
	void SelectObject(FName ObjectId);
	void RotatePreview();
	void ConfirmPlacement();
	void DeleteHoveredObject();

	//~ 카탈로그
	void SetCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog);
	UWjWorldPlaceableObjectDataAsset* GetCatalog() const { return Catalog; }

	//~ 저장/로드
	void SaveLayout();
	void LoadLayout();

	//~ 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementModeChanged OnPlacementModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnObjectPlaced OnObjectPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnObjectDeleted OnObjectDeleted;

	//~ 입력 에셋 (에디터에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlacementMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ConfirmAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CancelAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DeleteAction;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 마우스→월드 라인트레이스 (바닥 탐색) */
	bool TraceGroundUnderMouse(FHitResult& OutHit) const;

	/** 배치 위치 유효성 검사 (겹침 체크) */
	bool IsPlacementLocationValid(const FVector& Location) const;

	/** 삭제 타겟 탐색 (마우스 아래 PlacedObjectActor) */
	AWjWorldPlacedObjectActor* TraceForPlacedObject() const;

	/** 프리뷰 액터 제거 */
	void DestroyPreview();

	/** 입력 설정 로드 (DeveloperSettings에서) */
	void LoadInputSettings();

	/** Catalog가 설정되지 않았으면 DeveloperSettings에서 로드 (폴백) */
	void EnsureCatalogLoaded();

	/** 입력 바인딩 등록/해제 */
	void BindInputActions();
	void UnbindInputActions();

	/** 입력 콜백 */
	void OnConfirmAction(const FInputActionValue& Value);
	void OnCancelAction(const FInputActionValue& Value);
	void OnRotateAction(const FInputActionValue& Value);
	void OnDeleteAction(const FInputActionValue& Value);

	APlayerController* GetPlayerController() const;

	/** GameStateLobby 참조 (멀티플레이 동기화용) */
	AWjWorldGameStateLobby* GetLobbyGameState() const;

	/** 삭제 대상 오브젝트의 GameState 인덱스 찾기 */
	int32 FindPlacedObjectIndex(AWjWorldPlacedObjectActor* Actor) const;

	UPROPERTY()
	TObjectPtr<UWjWorldPlaceableObjectDataAsset> Catalog;

	UPROPERTY()
	TObjectPtr<AWjWorldPlacementPreviewActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<AWjWorldPlacedObjectActor> HoveredObject;

	FName SelectedObjectId;
	EPlacementMode CurrentMode = EPlacementMode::None;

	static const FString SaveSlotName;

	/** 현재 프리뷰 유효 상태 캐싱 */
	bool bCurrentPreviewValid = false;

	/** 트레이스 거리 */
	static constexpr float TraceDistance = 10000.0f;

	/** 겹침 체크 반경 */
	static constexpr float OverlapCheckRadius = 50.0f;
};
