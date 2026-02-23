// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Placement/WjWorldPlacedObjectActor.h"
#include "WjWorldTreasureChestActor.generated.h"

class UBoxComponent;
class UWidgetComponent;
class UStaticMeshComponent;
class UInteractionWidget;
class UInputAction;

/**
 * 보물상자 오브젝트 (로비 레벨에 개발자가 직접 배치)
 * 플레이어가 접근하여 F키로 열면 Coin 보상 획득
 * Steam: TriggerItemDrop으로 보상 + 서버 사이드 쿨타임 강제
 * 비Steam: GrantCurrencyLocally + GConfig 로컬 쿨타임 (에디터 테스트용)
 */
UCLASS()
class WJWORLD_API AWjWorldTreasureChestActor : public AWjWorldPlacedObjectActor
{
	GENERATED_BODY()

public:
	AWjWorldTreasureChestActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

private:
	// === Components ===

	/** 뚜껑 메시 (Roll 회전으로 열림/닫힘 연출) */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LidMeshComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractionTrigger;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> InteractionWidgetComponent;

	// === Settings (레벨에서 인스턴스별 설정) ===

	/** 상자 고유 인덱스 (Steam DefId = StartDefId + ChestIndex) */
	UPROPERTY(EditAnywhere, Category = "TreasureChest", meta = (ClampMin = "0"))
	int32 ChestIndex = 0;

	// === State ===

	UPROPERTY()
	TObjectPtr<UInteractionWidget> InteractionWidget;

	UPROPERTY()
	TObjectPtr<APawn> InteractingPlayer;

	bool bPlayerInRange = false;
	bool bInputBound = false;

	FTimerHandle CooldownUITimerHandle;

	// === Events ===

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// === Interaction ===

	UFUNCTION()
	void OnInteract();

	void ShowInteractionUI();
	void HideInteractionUI();
	void UpdateInteractionUI();

	// === Reward ===

	/** Steam TriggerItemDrop 또는 비Steam 로컬 보상 지급. 성공 시 true 반환 */
	bool TryGrantReward();

	// === Cooldown (per-player, 로컬 UI 표시용 + 비Steam 폴백) ===

	FString GetChestKey() const;
	bool IsOnCooldown() const;
	FTimespan GetRemainingCooldown() const;
	void SaveCooldown();

	// === Lid Animation ===

	/** 뚜껑 열기/닫기 애니메이션 시작 */
	void PlayLidOpen();
	void PlayLidClose();

	/** 뚜껑을 즉시 열린/닫힌 상태로 설정 (애니메이션 없이) */
	void SetLidOpenImmediate();
	void SetLidCloseImmediate();

	/** 뚜껑 애니메이션 상태 */
	bool bLidAnimating = false;
	bool bLidOpening = false;
	float LidCurrentRoll = 0.0f;

	static constexpr float LidClosedRoll = 0.0f;
	static constexpr float LidOpenedRoll = -120.0f;
	static constexpr float LidAnimSpeed = 200.0f; // 도/초

	// === Visual ===

	void ApplyCooldownVisual();
	void RemoveCooldownVisual();

public:
	/** 쿨타임 초기화 (테스트용) */
	void ResetCooldown();

private:

	// === Config ===

	static const FString ConfigSection;
	FString GetConfigPath() const;

	/** 쿨타임 인메모리 캐시 (GConfig read-back 불안정 해결) */
	FDateTime CachedLastOpenedTime;
};
