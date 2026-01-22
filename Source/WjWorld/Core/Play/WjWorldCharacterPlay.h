// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "WjWorldCharacterPlay.generated.h"

class UWjWorldAbilitySystemComponent;
class UCharacterPlaySetupDataAsset;
class UBoxComponent;

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

	// 히트박스 컴포넌트 (벽돌과 충돌 감지용)
	UFUNCTION(BlueprintCallable, Category = "Collision")
	UBoxComponent* GetHitBox() const { return HitBoxComponent; }

protected:
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	void OnRep_IsEliminated();

	// 사망 시 클라이언트에서 호출되는 로직
	void HandleEliminationEffects();

private:
	TWeakObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, category = "DataAsset")
	TSoftObjectPtr<UCharacterPlaySetupDataAsset> SetupDataAsset;

	// 히트박스 (벽돌과 충돌 감지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> HitBoxComponent;

	// 사망 상태 (리플리케이션)
	UPROPERTY(ReplicatedUsing = OnRep_IsEliminated)
	bool bIsEliminated = false;
};
