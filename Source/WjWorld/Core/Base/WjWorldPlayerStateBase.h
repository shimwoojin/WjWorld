// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Cosmetic/WjWorldCosmeticTypes.h"
#include "WjWorldPlayerStateBase.generated.h"

/**
 * 플레이어 상태 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerReadyStateChanged, int32, PlayerID, bool, bIsReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameUpdated, const FString&, PlayerName);

/**
 * 기본 플레이어 상태 클래스
 *
 * 역할:
 * - 플레이어 준비 상태 관리 (Replicated)
 * - 플레이어 정보 저장
 * - 코스메틱 로드아웃 관리 (Replicated)
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:
	AWjWorldPlayerStateBase();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ Ready State
	/**
	 * 준비 상태 토글 (Client → Server RPC)
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void ToggleReady();

	/**
	 * 준비 상태 설정 (Server Only)
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetReady(bool bNewReady);

	/**
	 * 준비 상태 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "PlayerState")
	bool IsReady() const { return bIsReady; }

	// ---- 코스메틱 로드아웃 ----

	/** 코스메틱 로드아웃 설정 (서버에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	void SetCosmeticLoadout(const FCosmeticLoadout& InLoadout);

	/** 현재 코스메틱 로드아웃 반환 */
	UFUNCTION(BlueprintPure, Category = "Cosmetic")
	const FCosmeticLoadout& GetCosmeticLoadout() const { return CosmeticLoadout; }

	//~ Events
	UPROPERTY(BlueprintAssignable, Category = "PlayerState|Events")
	FOnPlayerReadyStateChanged OnReadyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState|Events")
	FOnPlayerNameUpdated OnPlayerNameUpdated;

protected:
	/** 서브클래스에서 코스메틱 로드아웃 업데이트 시 호출되는 훅 */
	virtual void OnCosmeticLoadoutUpdated();

	UFUNCTION()
	void OnRep_CosmeticLoadout();

	//~ Replicated Properties
	/**
	 * 준비 상태 (Server → All Clients)
	 */
	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "PlayerState")
	bool bIsReady = false;

	//~ Replication Callbacks
	UFUNCTION()
	void OnRep_IsReady();

	//~ Override APlayerState
	virtual void OnRep_PlayerName() override;

	//~ Server RPC
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetCosmeticLoadout(const FCosmeticLoadout& InLoadout);

private:
	/** 리플리케이션되는 코스메틱 로드아웃 */
	UPROPERTY(ReplicatedUsing = OnRep_CosmeticLoadout)
	FCosmeticLoadout CosmeticLoadout;
};
