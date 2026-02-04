// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "CosmeticPreviewPanel.generated.h"

class UImage;
class UButton;
class ACharacterPreviewActor;

/**
 * 3D 캐릭터 프리뷰 패널
 *
 * CharacterPreviewActor를 재사용하여 코스메틱 시착 프리뷰 표시
 * - 현재 로드아웃 표시
 * - 아이템 시착 (임시 적용)
 * - 시착 취소 (원본 복원)
 */
UCLASS()
class WJWORLD_API UCosmeticPreviewPanel : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 현재 로드아웃으로 프리뷰 설정 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ShowCurrentLoadout();

	/** 아이템 시착 (임시 적용) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void PreviewItem(ECosmeticSlot CosmeticSlot, FName ItemId);

	/** 시착 취소 (원본 복원) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void ResetPreview();

	/** 프리뷰 액터를 특정 각도로 회전 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void RotatePreview(float DeltaYaw);

protected:
	/** 프리뷰 이미지 (RenderTarget 표시) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PreviewImage;

	/** 왼쪽 회전 버튼 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RotateLeftButton;

	/** 오른쪽 회전 버튼 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RotateRightButton;

	/** 프리뷰 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<ACharacterPreviewActor> PreviewActorClass;

	UFUNCTION()
	void OnRotateLeftClicked();

	UFUNCTION()
	void OnRotateRightClicked();

private:
	/** 프리뷰 액터 생성 */
	void CreatePreviewActor();

	/** 프리뷰 액터 파괴 */
	void DestroyPreviewActor();

	/** RenderTarget을 이미지에 적용 */
	void ApplyRenderTargetToImage();

	/** 프리뷰 액터 인스턴스 */
	UPROPERTY()
	TObjectPtr<ACharacterPreviewActor> PreviewActor;

	/** 원본 로드아웃 (시착 취소용) */
	FCosmeticLoadout OriginalLoadout;

	/** 현재 시착 중인 로드아웃 */
	FCosmeticLoadout PreviewLoadout;

	/** 시착 중 여부 */
	bool bIsTryingOn = false;

	/** 회전 각도 (도) */
	static constexpr float RotateAngle = 45.f;
};
