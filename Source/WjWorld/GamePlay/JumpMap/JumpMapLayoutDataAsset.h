// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JumpMapLayoutDataAsset.generated.h"

/**
 * JumpMap 레이아웃 오브젝트 엔트리
 */
USTRUCT(BlueprintType)
struct FJumpMapLayoutEntry
{
	GENERATED_BODY()

	/** 오브젝트 ID (카탈로그 참조용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ObjectId;

	/** 월드 트랜스폼 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform Transform;

	/** 액터별 커스텀 프로퍼티 (Key=Value 쌍) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, FString> CustomProperties;
};

/**
 * JumpMap 레이아웃 (오브젝트 집합)
 */
USTRUCT(BlueprintType)
struct FJumpMapLayout
{
	GENERATED_BODY()

	/** 레이아웃 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString LayoutName;

	/** 레이아웃에 포함된 오브젝트들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FJumpMapLayoutEntry> Objects;
};

/**
 * JumpMap 레이아웃 데이터 에셋
 * - 내장 레이아웃 + 유저 레이아웃(CSV) 지원
 * - WjWorldWallDescriptionDataAsset 패턴 참고
 */
UCLASS()
class WJWORLD_API UJumpMapLayoutDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 내장 레이아웃 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpMap Layout")
	TArray<FJumpMapLayout> BuiltInLayouts;

	/** 유저 레이아웃 디렉토리 스캔 */
	int32 ScanUserJumpMapLayouts(TArray<FJumpMapLayout>& OutUserLayouts) const;

	/** 이름으로 내장 레이아웃 검색 */
	const FJumpMapLayout* GetLayoutByName(const FString& Name) const;

	/** 이름으로 검색 (유저 레이아웃 포함) */
	const FJumpMapLayout* GetLayoutByNameIncludingUser(const FString& Name) const;

	/** 모든 레이아웃 이름 반환 (내장 + 유저) */
	TArray<FString> GetAllLayoutNames() const;

	/** 랜덤 레이아웃 이름 반환 */
	FString GetRandomLayoutName() const;

	/** 레이아웃을 CSV 문자열로 내보내기 */
	static FString ExportLayoutToCSV(const FJumpMapLayout& Layout);

private:
	/** CSV 파일 내용을 레이아웃으로 파싱 */
	bool ParseLayoutCSV(const FString& FileContent, FJumpMapLayout& OutLayout) const;

	/** 유저 레이아웃 디렉토리 경로 */
	static FString GetUserJumpMapLayoutDirectory();

	/** 캐시된 유저 레이아웃 */
	mutable TArray<FJumpMapLayout> CachedUserLayouts;
	mutable bool bUserLayoutsCached = false;
};
