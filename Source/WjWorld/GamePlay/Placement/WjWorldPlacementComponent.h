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
	EPlacementMode GetCurrentMode() const { return CurrentMode; }
	EPlacementContext GetCurrentContext() const { return CurrentContext; }

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

	/** 슬롯 이름을 지정하여 저장 */
	void SaveLayoutToSlot(const FString& SlotName);

	/** 슬롯 이름을 지정하여 로드 */
	bool LoadLayoutFromSlot(const FString& SlotName);

	/** 저장된 슬롯 목록 가져오기 */
	TArray<FString> GetSavedLayoutSlots() const;

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
	 * AW 컨텍스트: 마지막으로 내보낸 CSV 파일 경로 반환
	 */
	FString GetLastExportedCSVPath() const { return LastExportedCSVPath; }

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

	/** AW 컨텍스트: 마지막으로 내보낸 CSV 경로 */
	FString LastExportedCSVPath;

	/** 트레이스 거리 */
	static constexpr float TraceDistance = 10000.0f;

	/** 겹침 체크 반경 (일반 컨텍스트) */
	static constexpr float OverlapCheckRadius = 50.0f;

	/** 겹침 체크 반경 (AW 그리드 스냅 컨텍스트) - 정확히 같은 위치만 체크 */
	static constexpr float GridOverlapCheckRadius = 5.0f;
};
