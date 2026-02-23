// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "WjWorldPlacementComponent.generated.h"

class AWjWorldPlacementPreviewActor;
class AWjWorldPlacedObjectActor;
class AWjWorldGameStateLobby;
class UWjWorldPlaceableObjectDataAsset;
class UInputMappingContext;
class UInputAction;
class IWjWorldPlacementDataProvider;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAirModeChanged, bool, bIsAirMode);

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

	/** 배치 모드 진입 (기본 컨텍스트: Lobby) */
	void EnterPlacementMode();

	/** 특정 컨텍스트로 배치 모드 진입 */
	void EnterPlacementModeWithContext(EPlacementContext Context);

	void ExitPlacementMode();
	void ToggleDeleteMode();
	void ToggleAirPlacementMode();
	bool IsAirPlacementMode() const { return bAirPlacementMode; }
	float GetAirPlaneHeight() const { return AirPlaneHeight; }
	EPlacementMode GetCurrentMode() const { return CurrentMode; }
	EPlacementContext GetCurrentContext() const { return CurrentContext; }

	//~ 오브젝트 조작
	void SelectObject(FName ObjectId);
	void RotatePreview();
	void CycleRotationAxis();
	void CycleSnapDegrees();
	void ConfirmPlacement();
	void DeleteHoveredObject();

	/** 배치된 모든 오브젝트 삭제 + 저장 + 델리게이트 */
	void ClearAllPlacedObjects();

	/** 특정 ObjectId의 현재 배치 수 반환 */
	int32 CountPlacedObjectsByType(FName ObjectId) const;

	//~ 카탈로그
	void SetCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog);
	UWjWorldPlaceableObjectDataAsset* GetCatalog() const { return Catalog; }

	//~ 저장/로드
	void SaveLayout();
	void LoadLayout();

	/** 슬롯 이름을 지정하여 저장 */
	void SaveLayoutToSlot(const FString& SlotName);

	/** 슬롯 이름을 지정하여 로드 */
	bool LoadLayoutFromSlot(const FString& SlotName);

	/** 저장된 슬롯 목록 가져오기 */
	TArray<FString> GetSavedLayoutSlots() const;

	/** 저장된 레이아웃 슬롯 삭제 */
	bool DeleteLayoutSlot(const FString& SlotName);

	/** 현재 로드된 슬롯 이름 반환 (없으면 빈 문자열) */
	FString GetLoadedSlotName() const { return LoadedSlotName; }

	/** 현재 로드된 슬롯 이름 설정 (UI에서 저장 시 기본값으로 사용) */
	void SetLoadedSlotName(const FString& SlotName) { LoadedSlotName = SlotName; }

	/** 배치 데이터 프로바이더 참조 반환 (UI에서 사용) */
	IWjWorldPlacementDataProvider* GetPlacementDataProvider() const;

	/**
	 * AW 컨텍스트: 배치된 오브젝트를 CSV 벽 레이아웃으로 내보내기
	 * @param FileName 파일 이름 (확장자 제외)
	 * @return 내보내기 성공 여부
	 */
	bool ExportLayoutAsCSV(const FString& FileName);

	/**
	 * JumpMap 컨텍스트: 배치된 오브젝트를 CSV로 내보내기
	 * @param FileName 파일 이름 (확장자 제외)
	 * @return 내보내기 성공 여부
	 */
	bool ExportJumpMapLayoutAsCSV(const FString& FileName);

	/**
	 * AW 컨텍스트: 마지막으로 내보낸 CSV 파일 경로 반환
	 */
	FString GetLastExportedCSVPath() const { return LastExportedCSVPath; }

	/**
	 * JumpMap 레이아웃 검증 (저장 전 호출)
	 * @param OutErrorMessage 검증 실패 시 오류 메시지
	 * @return 검증 통과 여부
	 */
	bool ValidateJumpMapLayout(FString& OutErrorMessage) const;

	//~ 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementModeChanged OnPlacementModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnObjectPlaced OnObjectPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnObjectDeleted OnObjectDeleted;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnAirModeChanged OnAirModeChanged;

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

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ScrollAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleAirModeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CycleAxisAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CycleSnapAction;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 마우스→월드 라인트레이스 (바닥 탐색) */
	bool TraceGroundUnderMouse(FHitResult& OutHit) const;

	/** Air 모드: 수평 평면(Z=AirPlaneHeight)과 레이 교차점 계산 */
	bool TraceAirPlane(FVector& OutLocation) const;

	/** 배치 위치 유효성 검사 (겹침 체크) */
	bool IsPlacementLocationValid(const FVector& Location) const;

	/** AW 컨텍스트: 그리드 스냅 적용 */
	FVector SnapToGrid(const FVector& Location) const;

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
	void OnScrollAction(const FInputActionValue& Value);
	void OnToggleAirModeAction(const FInputActionValue& Value);
	void OnCycleAxisAction(const FInputActionValue& Value);
	void OnCycleSnapAction(const FInputActionValue& Value);

	/** 3D 회전 축 기즈모 그리기 */
	void DrawRotationAxisGizmo() const;

	/** 배치 모드 비주얼 갱신 (모든 PlacedObjectActor) */
	void RefreshPlacementModeVisuals();

	/** 현재 유효 스냅 각도 반환 */
	float GetEffectiveSnapDegrees() const;

	APlayerController* GetPlayerController() const;

	/** @deprecated GetPlacementDataProvider 사용 권장 */
	AWjWorldGameStateLobby* GetLobbyGameState() const;

	/** 삭제 대상 오브젝트의 GameState 인덱스 찾기 */
	int32 FindPlacedObjectIndex(AWjWorldPlacedObjectActor* Actor) const;

	/** 현재 컨텍스트의 SaveSlot 이름 반환 */
	FString GetCurrentSaveSlotName() const;

	UPROPERTY()
	TObjectPtr<UWjWorldPlaceableObjectDataAsset> Catalog;

	UPROPERTY()
	TObjectPtr<AWjWorldPlacementPreviewActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<AWjWorldPlacedObjectActor> HoveredObject;

	FName SelectedObjectId;
	EPlacementMode CurrentMode = EPlacementMode::None;
	EPlacementContext CurrentContext = EPlacementContext::Lobby;

	/** 현재 로드된 슬롯 이름 (저장 시 기본값으로 사용) */
	FString LoadedSlotName;

	/** 현재 프리뷰 유효 상태 캐싱 */
	bool bCurrentPreviewValid = false;

	/** Air 배치 모드 활성 여부 */
	bool bAirPlacementMode = false;

	/** Air 모드 수평 평면 높이 (Z) */
	float AirPlaneHeight = 0.f;

	/** AW 컨텍스트: 마지막으로 내보낸 CSV 경로 */
	FString LastExportedCSVPath;

	/** Air 모드 스크롤 1단위당 높이 변화 */
	static constexpr float AirHeightStep = 50.f;

	/** Air 모드 최대 레이-평면 교차 거리 */
	static constexpr float MaxAirTraceDistance = 50000.f;

	/** 트레이스 거리 */
	static constexpr float TraceDistance = 10000.0f;

	/** 겹침 체크 반경 (일반 컨텍스트) */
	static constexpr float OverlapCheckRadius = 50.0f;

	/** 겹침 체크 반경 (AW 그리드 스냅 컨텍스트) - 정확히 같은 위치만 체크 */
	static constexpr float GridOverlapCheckRadius = 5.0f;

	/** 스냅 각도 프리셋 */
	static constexpr float SnapDegreePresets[] = { 90.f, 45.f, 15.f, 5.f, 1.f };
	static constexpr int32 NumSnapPresets = UE_ARRAY_COUNT(SnapDegreePresets);

	/** 현재 스냅 프리셋 인덱스 */
	int32 CurrentSnapPresetIndex = 0;

	/** 기즈모 화살표 길이 */
	static constexpr float GizmoArrowLength = 150.f;

	/** 기즈모 화살표 크기 */
	static constexpr float GizmoArrowSize = 25.f;

	/** 기즈모 화살표 두께 */
	static constexpr float GizmoArrowThickness = 3.f;
};
