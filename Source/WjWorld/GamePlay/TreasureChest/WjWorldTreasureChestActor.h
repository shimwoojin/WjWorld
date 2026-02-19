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
 * 보물상자 배치 오브젝트
 * 플레이어가 접근하여 F키로 열면 Coin 보상 획득, 쿨타임 적용
 * 각 플레이어마다 독립적인 상태 (로컬 GConfig 저장)
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

	// === State ===

	UPROPERTY()
	TObjectPtr<UInteractionWidget> InteractionWidget;

	UPROPERTY()
	TObjectPtr<APawn> InteractingPlayer;

	bool bPlayerInRange = false;

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

	// === Cooldown (per-player, GConfig) ===

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

	// === Config ===

	static const FString ConfigSection;
	FString GetConfigPath() const;
};
