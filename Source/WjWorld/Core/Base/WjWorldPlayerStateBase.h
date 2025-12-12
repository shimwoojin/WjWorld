// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
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
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerStateBase : public APlayerState
{
	GENERATED_BODY()
	
public:
	AWjWorldPlayerStateBase();

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

	//~ Events
	UPROPERTY(BlueprintAssignable, Category = "PlayerState|Events")
	FOnPlayerReadyStateChanged OnReadyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState|Events")
	FOnPlayerNameUpdated OnPlayerNameUpdated;

protected:
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
};
