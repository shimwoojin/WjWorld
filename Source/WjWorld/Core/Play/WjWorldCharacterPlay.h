// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "WjWorldCharacterPlay.generated.h"

class UWjWorldAbilitySystemComponent;
class UWjWorldCosmeticComponent;
class UCharacterPlaySetupDataAsset;
class UBoxComponent;

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

	UFUNCTION(BlueprintCallable, Category = "Death")
	int32 GetDeadStackCount() const { return DeadStackCount; }

	UFUNCTION(BlueprintCallable, Category = "Death")
	void AddDeadStackCount(int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Death")
	void RemoveDeadStackCount(int32 InCount);

protected:
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	void OnRep_IsEliminated();

	UFUNCTION()
	void OnRep_DeadStackCountChanged();

	// 사망 시 클라이언트에서 호출되는 로직
	void HandleEliminationEffects();

private:
	UFUNCTION()
	void OnSetupDataAssetLoaded(const FSoftObjectPath& Path, UObject* Object);

	virtual void GasInputPressed(int32 InputID) override;
	virtual void GasInputReleased(int32 InputID) override;

public:
	FOnDeadStackCounthanged OnDeadStackCountChanged;

private:
	TWeakObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cosmetic", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldCosmeticComponent> CosmeticComponent;

	UPROPERTY(EditDefaultsOnly, category = "DataAsset", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UCharacterPlaySetupDataAsset> SetupDataAsset;

	// 사망 상태 (리플리케이션)
	UPROPERTY(ReplicatedUsing = OnRep_IsEliminated)
	bool bIsEliminated = false;

	UPROPERTY(ReplicatedUsing = OnRep_DeadStackCountChanged)
	int32 DeadStackCount;
};
