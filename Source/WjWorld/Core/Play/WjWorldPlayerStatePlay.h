// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "AbilitySystemInterface.h"
#include "WjWorldPlayerStatePlay.generated.h"

class UWjWorldAbilitySystemComponent;
class UWjWorldGameDataComponent;

/**
 * 
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

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated)
	TObjectPtr<UWjWorldGameDataComponent> PlayerDataComponent;

};
