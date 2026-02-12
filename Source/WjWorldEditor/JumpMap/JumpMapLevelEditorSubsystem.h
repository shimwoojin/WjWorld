// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "JumpMapLevelEditorSubsystem.generated.h"

class AJumpMapActorBase;

/**
 * JumpMap 레벨 에디터 서브시스템
 * - 현재 레벨의 JumpMap 액터 일괄 저장/불러오기
 * - CSV 기반 레이아웃 관리
 */
UCLASS()
class WJWORLDEDITOR_API UJumpMapLevelEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/** 현재 레벨의 JumpMap 액터를 CSV로 저장 */
	UFUNCTION(BlueprintCallable, Category = "JumpMap Editor")
	bool SaveCurrentLevelLayout(const FString& LayoutName);

	/** CSV에서 레이아웃 로드 → 현재 레벨에 액터 스폰 */
	UFUNCTION(BlueprintCallable, Category = "JumpMap Editor")
	bool LoadLayoutToCurrentLevel(const FString& LayoutName);

	/** 현재 레벨의 모든 JumpMap 액터 제거 */
	UFUNCTION(BlueprintCallable, Category = "JumpMap Editor")
	void ClearAllJumpMapActors();

	/** 사용 가능한 레이아웃 목록 (유저 CSV) */
	UFUNCTION(BlueprintCallable, Category = "JumpMap Editor")
	TArray<FString> GetAvailableLayoutNames() const;

	/** 현재 레벨의 JumpMap 액터 수 */
	UFUNCTION(BlueprintCallable, Category = "JumpMap Editor")
	int32 GetJumpMapActorCount() const;

private:
	/** 유저 레이아웃 저장 디렉토리 */
	static FString GetUserLayoutDirectory();

	/** ObjectId → ActorClass 매핑 로드 (DeveloperSettings) */
	TSubclassOf<AJumpMapActorBase> GetActorClassForObjectId(const FName& ObjectId) const;
};
