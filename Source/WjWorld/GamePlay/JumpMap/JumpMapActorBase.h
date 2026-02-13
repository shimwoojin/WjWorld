// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpMapActorBase.generated.h"

/**
 * JumpMap 게임플레이 액터 베이스 클래스
 * - 배치 시스템으로 스폰되는 모든 JumpMap 오브젝트의 기반
 */
UCLASS(Abstract)
class WJWORLD_API AJumpMapActorBase : public AActor
{
	GENERATED_BODY()

public:
	AJumpMapActorBase();

	/** 레이아웃 엔트리로부터 초기화 (트랜스폼 적용) */
	virtual void InitializeFromLayoutEntry(const FTransform& InTransform);

	/** 이 액터의 레이아웃 ObjectId 반환 */
	FName GetJumpMapObjectId() const { return JumpMapObjectId; }

	/** 커스텀 프로퍼티를 맵으로 직렬화 */
	virtual void GetSerializableProperties(TMap<FString, FString>& OutProperties) const {}

	/** 맵에서 커스텀 프로퍼티 적용 */
	virtual void ApplySerializedProperties(const TMap<FString, FString>& Properties) {}

	/** 기본 배치 액터 여부 (레이아웃 저장/삭제 대상에서 제외) */
	UPROPERTY(EditInstanceOnly, Category = "JumpMap")
	bool bIsDefaultPlacement = false;

protected:
	/** 이 액터의 레이아웃 ObjectId (서브클래스 생성자에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "JumpMap")
	FName JumpMapObjectId;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
