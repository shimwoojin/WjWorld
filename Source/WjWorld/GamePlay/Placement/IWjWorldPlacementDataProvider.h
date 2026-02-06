// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IWjWorldPlacementDataProvider.generated.h"

struct FPlacedObjectSaveEntry;
class UWjWorldPlaceableObjectDataAsset;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UWjWorldPlacementDataProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 배치 데이터 제공자 인터페이스
 * GameState 클래스에서 구현하여 배치 데이터 관리 및 리플리케이션 지원
 */
class WJWORLD_API IWjWorldPlacementDataProvider
{
	GENERATED_BODY()

public:
	/**
	 * 배치 오브젝트 추가 (서버에서 호출)
	 * @param Entry 배치할 오브젝트 정보
	 */
	virtual void AddPlacedObject(const FPlacedObjectSaveEntry& Entry) = 0;

	/**
	 * 배치 오브젝트 제거 (서버에서 호출)
	 * @param Index 제거할 오브젝트의 인덱스
	 */
	virtual void RemovePlacedObjectAt(int32 Index) = 0;

	/**
	 * 전체 배치 오브젝트 목록 반환
	 * @return 배치 오브젝트 배열 참조
	 */
	virtual const TArray<FPlacedObjectSaveEntry>& GetPlacedObjects() const = 0;

	/**
	 * 배치 오브젝트 일괄 설정 (기존 오브젝트 모두 교체)
	 * @param InPlacedObjects 새로운 배치 오브젝트 목록
	 */
	virtual void SetPlacedObjects(const TArray<FPlacedObjectSaveEntry>& InPlacedObjects) = 0;

	/**
	 * 모든 배치 오브젝트 제거
	 */
	virtual void ClearPlacedObjects() = 0;

	/**
	 * 카탈로그 설정
	 * @param InCatalog 사용할 카탈로그
	 */
	virtual void SetPlacementCatalog(UWjWorldPlaceableObjectDataAsset* InCatalog) = 0;

	/**
	 * 현재 카탈로그 반환
	 * @return 현재 설정된 카탈로그
	 */
	virtual UWjWorldPlaceableObjectDataAsset* GetPlacementCatalog() const = 0;
};
