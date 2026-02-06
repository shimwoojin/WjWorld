// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FPlacedObjectSaveEntry;
struct FWjWorldWallDescription;

/**
 * 배치된 오브젝트들을 WallLayout 포맷으로 변환하는 유틸리티
 */
class WJWORLD_API FWjWorldWallLayoutConverter
{
public:
	/**
	 * 배치된 오브젝트들을 WallLayout 2D 배열로 변환
	 * @param PlacedObjects - 배치된 오브젝트 목록
	 * @param BrickSize - 벽돌 크기 (그리드 셀 크기)
	 * @param OutWallLayout - 결과 2D 배열 (Row, Column)
	 * @param OutRowNum - 결과 행 수
	 * @param OutColumnNum - 결과 열 수
	 * @param OutCenterOffset - 결과 중심 오프셋
	 * @return 변환 성공 여부
	 */
	static bool ConvertPlacedObjectsToWallLayout(
		const TArray<FPlacedObjectSaveEntry>& PlacedObjects,
		const FVector& BrickSize,
		TArray<TArray<int32>>& OutWallLayout,
		int32& OutRowNum,
		int32& OutColumnNum,
		FVector& OutCenterOffset
	);

	/**
	 * WallLayout을 CSV 문자열로 변환
	 * @param WallLayout - 2D 배열
	 * @return CSV 포맷 문자열
	 */
	static FString WallLayoutToCSVString(const TArray<TArray<int32>>& WallLayout);

	/**
	 * WallLayout의 유효성 검사 (빈 영역이 벽으로 둘러싸여 있는지)
	 * @param WallLayout - 2D 배열
	 * @param OutErrorMessage - 에러 메시지 (유효하지 않은 경우)
	 * @return 유효 여부
	 */
	static bool ValidateWallLayout(
		const TArray<TArray<int32>>& WallLayout,
		FText& OutErrorMessage
	);

	/**
	 * CSV 파일로 저장
	 * @param WallLayout - 2D 배열
	 * @param FilePath - 저장 경로
	 * @return 저장 성공 여부
	 */
	static bool SaveWallLayoutToFile(
		const TArray<TArray<int32>>& WallLayout,
		const FString& FilePath
	);

private:
	static constexpr int32 WallCell_Empty = -1;
	static constexpr int32 WallCell_Wall = 1;

	/**
	 * 경계에서 Flood Fill로 외부 영역(경계에 연결된 빈 셀) 마킹
	 * @param WallLayout - 2D 배열
	 * @param OutExteriorCells - 외부 영역 셀 집합
	 */
	static void MarkExteriorCells(
		const TArray<TArray<int32>>& WallLayout,
		TSet<FIntPoint>& OutExteriorCells
	);

	/**
	 * 외부 영역이 아닌 내부 빈 셀 찾기
	 * @param WallLayout - 2D 배열
	 * @param ExteriorCells - 외부 영역 셀 집합
	 * @param OutX, OutY - 찾은 내부 빈 셀 좌표
	 * @return 내부 빈 셀 존재 여부
	 */
	static bool FindInteriorEmptyCell(
		const TArray<TArray<int32>>& WallLayout,
		const TSet<FIntPoint>& ExteriorCells,
		int32& OutX,
		int32& OutY
	);
};
