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

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
