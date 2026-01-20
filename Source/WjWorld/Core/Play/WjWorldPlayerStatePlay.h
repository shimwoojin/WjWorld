// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "AbilitySystemInterface.h"
#include "WjWorldPlayerStatePlay.generated.h"

class UWjWorldAbilitySystemComponent;

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

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;
};
