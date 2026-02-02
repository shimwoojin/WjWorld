// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "AbilitySystemInterface.h"
#include "WjWorldPlayerStatePlay.generated.h"

class UWjWorldAbilitySystemComponent;
class UWjWorldGameDataComponent;
class UWjWorldCharacterAttributeSet;

/**
 * 게임플레이 플레이어 상태
 * - ASC (Ability System Component) 소유
 * - 코스메틱 로드아웃은 부모(PlayerStateBase)에서 관리
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerStatePlay : public AWjWorldPlayerStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AWjWorldPlayerStatePlay();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UWjWorldAbilitySystemComponent* GetWJAbilitySystemComponent() const;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

    // 게임별 데이터는 컴포넌트로
    template<typename T>
    T* GetGameData() const { return FindComponentByClass<T>(); }

	void AddGameDataComponent(TSubclassOf<UWjWorldGameDataComponent> InDataComponentClass);

protected:
	/** 코스메틱 로드아웃 업데이트 시 CharacterPlay의 CosmeticComponent에 적용 */
	virtual void OnCosmeticLoadoutUpdated() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldCharacterAttributeSet> CharacterAttributeSet;

	UPROPERTY(Replicated)
	TObjectPtr<UWjWorldGameDataComponent> PlayerDataComponent;
};
