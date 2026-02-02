// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "PlayerProfileWidget.generated.h"

class UImage;
class UTextBlock;
class UVerticalBox;
class UButton;
class ACharacterPreviewActor;

/**
 * 플레이어 프로필 위젯
 * - 3D 캐릭터 프리뷰 (코스메틱 포함)
 * - 플레이어 이름
 * - 미니게임별 스탯 표시
 */
UCLASS()
class WJWORLD_API UPlayerProfileWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 로컬 플레이어 프로필 표시 */
	UFUNCTION(BlueprintCallable, Category = "Profile")
	void ShowLocalProfile();

	/** 타 플레이어 프로필 표시 */
	UFUNCTION(BlueprintCallable, Category = "Profile")
	void ShowPlayerProfile(const FUniqueNetIdRepl& PlayerId,
		const FText& PlayerName,
		const FCosmeticLoadout& Loadout);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CharacterPreviewImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> StatsContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void OnCloseClicked();

	UFUNCTION()
	void OnUserStatsReceived(const FUniqueNetIdRepl& UserId);

	/** 스탯 섹션 UI 생성 */
	void PopulateLocalStats();
	void PopulateUserStats(const FUniqueNetIdRepl& UserId);

	/** 프리뷰 액터 생성/파괴 */
	void CreatePreviewActor(const FCosmeticLoadout& Loadout);
	void DestroyPreviewActor();

	/** 프리뷰 RenderTarget을 이미지에 적용 */
	void ApplyRenderTargetToImage();

	UPROPERTY()
	TObjectPtr<ACharacterPreviewActor> PreviewActor;

	/** 조회 중인 타 플레이어 ID */
	FUniqueNetIdRepl PendingUserId;
};
