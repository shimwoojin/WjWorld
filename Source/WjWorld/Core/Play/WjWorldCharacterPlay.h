// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "WjWorldCharacterPlay.generated.h"

class UWjWorldAbilitySystemComponent;
class UCharacterPlaySetupDataAsset;
class UBoxComponent;
class UWidgetComponent;
class UAbilityPromptWidget;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeadStackCounthanged, int32, NewDeadStackCount);

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldCharacterPlay : public AWjWorldCharacterBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AWjWorldCharacterPlay();

	/// UCharacterPlaySetupDataAsset::Initialize의 성공적인 호출 여부
	UPROPERTY()
	uint8 bSetupDataAssetCall:1;

	/// UCharacterPlaySetupDataAsset::Initialize 호출 이후 비동기 초기화까지 전부 확인된지 여부
	UPROPERTY()
	uint8 bSetupDataAssetFinal:1;

protected:
	virtual void PostInitializeComponents() override;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UWjWorldAbilitySystemComponent* GetWJAbilitySystemComponent() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 사망 처리
	UFUNCTION(BlueprintCallable, Category = "Death")
	void OnEliminated();

	UFUNCTION(BlueprintCallable, Category = "Death")
	bool IsEliminated() const { return bIsEliminated; }

	// 마지막 공격자 설정 (킬 스탯 추적용)
	void SetLastAttacker(AWjWorldCharacterPlay* Attacker);
	AWjWorldCharacterPlay* GetLastAttacker() const { return LastAttacker.Get(); }

	// 어빌리티 프롬프트 표시/숨김
	void ShowAbilityPrompt(const FText& ConfirmKeyName, const FText& CancelKeyName, const FText& Description);
	void HideAbilityPrompt();

	// 들고 있는 벽돌 시각화 (3자에게도 보임)
	UFUNCTION(BlueprintCallable, Category = "LiftBrick")
	void ShowLiftedBrick(UStaticMesh* BrickMesh, const FVector& BrickScale, const FLinearColor& BrickColor);

	UFUNCTION(BlueprintCallable, Category = "LiftBrick")
	void HideLiftedBrick();

	UFUNCTION(BlueprintCallable, Category = "LiftBrick")
	bool IsCarryingBrick() const { return bIsCarryingBrick; }

protected:
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	void OnRep_IsEliminated();

	UFUNCTION()
	void OnRep_IsCarryingBrick();

	// 사망 시 클라이언트에서 호출되는 로직
	void HandleEliminationEffects();

	// 들고 있는 벽돌 메시 업데이트 (로컬)
	void UpdateLiftedBrickVisual();

private:
	UFUNCTION()
	void OnSetupDataAssetLoaded(const FSoftObjectPath& Path, UObject* Object);

	virtual void GasInputPressed(int32 InputID) override;
	virtual void GasInputReleased(int32 InputID) override;

public:
	FOnDeadStackCounthanged OnDeadStackCountChanged;

private:
	TWeakObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, category = "DataAsset", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UCharacterPlaySetupDataAsset> SetupDataAsset;

	// 사망 상태 (리플리케이션)
	UPROPERTY(ReplicatedUsing = OnRep_IsEliminated)
	bool bIsEliminated = false;

	// 마지막 공격자 (킬 스탯 추적용)
	TWeakObjectPtr<AWjWorldCharacterPlay> LastAttacker;

	// 어빌리티 프롬프트 WidgetComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWidgetComponent> AbilityPromptComponent;

	// 들고 있는 벽돌 시각화 컴포넌트 (손 소켓에 부착)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LiftBrick", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> LiftedBrickMeshComponent;

	// 벽돌을 들고 있는지 여부 (3자 동기화)
	UPROPERTY(ReplicatedUsing = OnRep_IsCarryingBrick)
	bool bIsCarryingBrick = false;

	// 들고 있는 벽돌 메시 (리플리케이션용 캐시)
	UPROPERTY(Replicated)
	TObjectPtr<UStaticMesh> CarriedBrickMesh;

	// 들고 있는 벽돌 스케일 (리플리케이션용 캐시)
	UPROPERTY(Replicated)
	FVector CarriedBrickScale = FVector::OneVector;

	// 들고 있는 벽돌 색상 (리플리케이션용 캐시)
	UPROPERTY(Replicated)
	FLinearColor CarriedBrickColor = FLinearColor::White;

	// 벽돌 메시 Dynamic Material Instance (색상 적용용)
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> LiftedBrickDynamicMaterial;
};
