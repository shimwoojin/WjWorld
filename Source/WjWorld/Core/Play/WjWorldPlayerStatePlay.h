// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "AbilitySystemInterface.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "WjWorldPlayerStatePlay.generated.h"

class UWjWorldAbilitySystemComponent;
class UWjWorldGameDataComponent;
class UWjWorldCharacterAttributeSet;

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

	// ---- 코스메틱 로드아웃 ----

	/** 코스메틱 로드아웃 설정 (서버에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetCosmeticLoadout(const FCosmeticLoadout& InLoadout);

	/** 현재 코스메틱 로드아웃 반환 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	const FCosmeticLoadout& GetCosmeticLoadout() const { return CosmeticLoadout; }

protected:
	UFUNCTION()
	void OnRep_CosmeticLoadout();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UWjWorldCharacterAttributeSet> CharacterAttributeSet;

	UPROPERTY(Replicated)
	TObjectPtr<UWjWorldGameDataComponent> PlayerDataComponent;

	/** 리플리케이션되는 코스메틱 로드아웃 */
	UPROPERTY(ReplicatedUsing = OnRep_CosmeticLoadout)
	FCosmeticLoadout CosmeticLoadout;

};
