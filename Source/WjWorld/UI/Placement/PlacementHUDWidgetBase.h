// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"
#include "PlacementHUDWidgetBase.generated.h"

class UWjWorldPlacementComponent;
class UWjWorldPlaceableObjectDataAsset;
class UPlacementSaveDialogWidget;
class UPlacementLoadDialogWidget;
class UScrollBox;
class UButton;
class UTextBlock;

/**
 * 배치 모드 HUD 기본 클래스
 * 모든 컨텍스트(Lobby, AW Editor, JumpMap Editor)에서 공통으로 사용하는 기능 제공
 */
UCLASS(Abstract)
class WJWORLD_API UPlacementHUDWidgetBase : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** PlacementComponent 참조 설정 및 카탈로그 표시 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	virtual void SetPlacementComponent(UWjWorldPlacementComponent* InComponent);

	/** 현재 배치 컨텍스트 반환 */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	EPlacementContext GetCurrentContext() const;

	/** 저장 다이얼로그에서 확인 시 호출 (public으로 변경 - 델리게이트 바인딩용) */
	UFUNCTION()
	virtual void OnSaveConfirmed(const FString& SlotName);

protected:
	/** 카탈로그 스크롤 박스 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> CatalogScrollBox;

	/** 나가기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	/** 삭제 모드 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteModeButton;

	/** 저장 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SaveButton;

	/** 불러오기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadButton;

	/** 조작 안내 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ControlsHintText;

	/** 타이틀 텍스트 (선택적) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	/** 저장 다이얼로그 위젯 클래스 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TSubclassOf<UPlacementSaveDialogWidget> SaveDialogClass;

	/** 불러오기 다이얼로그 위젯 클래스 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TSubclassOf<UPlacementLoadDialogWidget> LoadDialogClass;

	/** 카탈로그 아이템 목록 채우기 */
	virtual void PopulateCatalog(UWjWorldPlaceableObjectDataAsset* Catalog);

	/** 나가기 버튼 클릭 핸들러 - 서브클래스에서 오버라이드 */
	UFUNCTION()
	virtual void OnExitClicked();

	/** 삭제 모드 버튼 클릭 핸들러 */
	UFUNCTION()
	virtual void OnDeleteModeClicked();

	/** 저장 버튼 클릭 핸들러 */
	UFUNCTION()
	virtual void OnSaveClicked();

	/** 불러오기 버튼 클릭 핸들러 */
	UFUNCTION()
	virtual void OnLoadClicked();

	/** 불러오기 다이얼로그에서 확인 시 호출 */
	UFUNCTION()
	virtual void OnLoadConfirmed(const FString& SlotName);

	/** 불러오기 다이얼로그에서 슬롯 삭제 요청 시 호출 */
	UFUNCTION()
	void OnSlotDeleteRequested(const FString& SlotName);

	/** 실제 저장 로직 - 서브클래스에서 오버라이드 가능 */
	virtual void ExecuteSave(const FString& SlotName);

	/** 실제 불러오기 로직 - 서브클래스에서 오버라이드 가능 */
	virtual void ExecuteLoad(const FString& SlotName);

	/** 현재 로드된 슬롯 이름 반환 (저장 시 기본값으로 사용) */
	FString GetCurrentLoadedSlotName() const;

	/** 카탈로그 버튼 클릭 공통 핸들러 */
	UFUNCTION()
	void OnCatalogButtonClicked();

	/** 카탈로그 항목 클릭 시 호출 */
	virtual void OnCatalogItemClicked(FName ObjectId);

	/** 타이틀 텍스트 설정 */
	void SetTitleText(const FText& Title);

	/** 조작 안내 텍스트 설정 */
	void SetControlsHintText(const FText& Hint);

	UPROPERTY()
	TObjectPtr<UWjWorldPlacementComponent> PlacementComponent;

	/** 버튼 → ObjectId 매핑 */
	UPROPERTY()
	TMap<TObjectPtr<UButton>, FName> ButtonToObjectIdMap;

	/** 저장 다이얼로그 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPlacementSaveDialogWidget> SaveDialogInstance;

	/** 불러오기 다이얼로그 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPlacementLoadDialogWidget> LoadDialogInstance;
};
