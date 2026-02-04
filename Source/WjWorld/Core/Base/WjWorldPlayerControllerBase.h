// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WjWorldPlayerControllerBase.generated.h"

class UInputMappingContext;
class UEnhancedInputComponent;

/**
 * 기본 플레이어 컨트롤러 클래스
 * 
 * 역할:
 * - 모든 플레이어 컨트롤러의 Base 클래스
 * - 공통 입력 처리 및 UI 관리
 */
UCLASS(abstract)
class WJWORLD_API AWjWorldPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	AWjWorldPlayerControllerBase();

protected:
	virtual void BeginPlay() override;

protected:
	/**
	 * 컨트롤러 초기화
	 */
	virtual void InitializeController();

	/**
	 * UI 초기화
	 */
	virtual void InitializeUI();

public:
	UFUNCTION(Exec)
	void CheckInputMode();

	UFUNCTION(Exec)
	void ChangeCharacterViewMode(int32 InViewMode);

	UFUNCTION(Exec)
	void ServerTravelWaitingRoom();

	// ---- 코스메틱 테스트 콘솔 명령어 ----

	/** 특정 아이템을 인벤토리에 추가 (테스트용) */
	UFUNCTION(Exec)
	void Cosmetic_GrantItem(FString ItemId);

	/** 모든 카탈로그 아이템을 인벤토리에 추가 */
	UFUNCTION(Exec)
	void Cosmetic_GrantAll();

	/** 인벤토리 초기화 */
	UFUNCTION(Exec)
	void Cosmetic_ClearInventory();

	/** 인벤토리 상태 출력 */
	UFUNCTION(Exec)
	void Cosmetic_PrintInventory();

	/** 로드아웃 상태 출력 */
	UFUNCTION(Exec)
	void Cosmetic_PrintLoadout();

	/** 특정 슬롯에 아이템 장착 (1=Head, 2=Body, 3=Back, 4=Effect) */
	UFUNCTION(Exec)
	void Cosmetic_Equip(int32 SlotIndex, FString ItemId);

	/** 특정 슬롯 장착 해제 */
	UFUNCTION(Exec)
	void Cosmetic_Unequip(int32 SlotIndex);

	/** Steam 인벤토리 갱신 요청 */
	UFUNCTION(Exec)
	void Cosmetic_RefreshInventory();

	/** Steam 프로모 아이템 지급 (SteamItemDefId 사용) */
	UFUNCTION(Exec)
	void Cosmetic_AddPromo(int32 SteamItemDefId);

	/** 모든 Steam 프로모 아이템 지급 */
	UFUNCTION(Exec)
	void Cosmetic_AddAllPromos();

	/** 코스메틱 상점 UI 열기 */
	UFUNCTION(Exec)
	void Cosmetic_OpenShop();
};
