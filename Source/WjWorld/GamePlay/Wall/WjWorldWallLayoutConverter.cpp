// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePlay/Wall/WjWorldWallLayoutConverter.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "Misc/FileHelper.h"
#include "WjWorldLogCategories.h"

bool FWjWorldWallLayoutConverter::ConvertPlacedObjectsToWallLayout(
	const TArray<FPlacedObjectSaveEntry>& PlacedObjects,
	const FVector& BrickSize,
	TArray<TArray<int32>>& OutWallLayout,
	int32& OutRowNum,
	int32& OutColumnNum,
	FVector& OutCenterOffset)
{
	OutWallLayout.Empty();
	OutRowNum = 0;
	OutColumnNum = 0;
	OutCenterOffset = FVector::ZeroVector;

	if (PlacedObjects.Num() == 0)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("WallLayoutConverter: No placed objects to convert"));
		return false;
	}

	if (BrickSize.X <= 0 || BrickSize.Y <= 0)
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("WallLayoutConverter: Invalid BrickSize"));
		return false;
	}

	// 1. 모든 배치된 오브젝트의 위치에서 그리드 좌표 범위 계산
	int32 MinGridX = MAX_int32, MaxGridX = MIN_int32;
	int32 MinGridY = MAX_int32, MaxGridY = MIN_int32;

	TArray<FIntPoint> GridPositions;
	GridPositions.Reserve(PlacedObjects.Num());

	for (const FPlacedObjectSaveEntry& Entry : PlacedObjects)
	{
		const FVector& Location = Entry.Transform.GetLocation();

		// 월드 좌표 -> 그리드 좌표 변환
		// X축 -> Column, Y축 -> Row로 매핑
		int32 GridX = FMath::RoundToInt(Location.X / BrickSize.X);
		int32 GridY = FMath::RoundToInt(Location.Y / BrickSize.Y);

		GridPositions.Add(FIntPoint(GridX, GridY));

		MinGridX = FMath::Min(MinGridX, GridX);
		MaxGridX = FMath::Max(MaxGridX, GridX);
		MinGridY = FMath::Min(MinGridY, GridY);
		MaxGridY = FMath::Max(MaxGridY, GridY);
	}

	// 2. 그리드 크기 계산 (1칸 여유 공간 추가 - 벽이 둘러싸기 위함)
	const int32 Padding = 1;
	OutColumnNum = (MaxGridX - MinGridX + 1) + (Padding * 2);
	OutRowNum = (MaxGridY - MinGridY + 1) + (Padding * 2);

	// 3. 빈 그리드 초기화 (-1 = 빈 셀)
	OutWallLayout.SetNum(OutRowNum);
	for (int32 Row = 0; Row < OutRowNum; ++Row)
	{
		OutWallLayout[Row].Init(WallCell_Empty, OutColumnNum);
	}

	// 4. 배치된 오브젝트 위치에 벽(1) 표시
	for (const FIntPoint& GridPos : GridPositions)
	{
		// 패딩 고려하여 오프셋 적용
		int32 AdjustedCol = (GridPos.X - MinGridX) + Padding;
		int32 AdjustedRow = (GridPos.Y - MinGridY) + Padding;

		if (OutWallLayout.IsValidIndex(AdjustedRow) && OutWallLayout[AdjustedRow].IsValidIndex(AdjustedCol))
		{
			OutWallLayout[AdjustedRow][AdjustedCol] = WallCell_Wall;
		}
	}

	// 5. 중심 오프셋 계산 (그리드 원점 -> 월드 좌표)
	OutCenterOffset.X = (MinGridX - Padding) * BrickSize.X;
	OutCenterOffset.Y = (MinGridY - Padding) * BrickSize.Y;
	OutCenterOffset.Z = 0.0f;

	UE_LOG(LogWjWorldPlacement, Log, TEXT("WallLayoutConverter: Converted %d objects to %dx%d grid"),
		PlacedObjects.Num(), OutRowNum, OutColumnNum);

	return true;
}

FString FWjWorldWallLayoutConverter::WallLayoutToCSVString(const TArray<TArray<int32>>& WallLayout)
{
	FString Result;

	for (int32 Row = 0; Row < WallLayout.Num(); ++Row)
	{
		const TArray<int32>& RowData = WallLayout[Row];
		for (int32 Col = 0; Col < RowData.Num(); ++Col)
		{
			Result += FString::Printf(TEXT("%d"), RowData[Col]);
			if (Col < RowData.Num() - 1)
			{
				Result += TEXT(",");
			}
		}
		Result += TEXT("\n");
	}

	return Result;
}

bool FWjWorldWallLayoutConverter::ValidateWallLayout(
	const TArray<TArray<int32>>& WallLayout,
	FText& OutErrorMessage)
{
	if (WallLayout.Num() == 0)
	{
		OutErrorMessage = FText::FromString(TEXT("레이아웃이 비어있습니다."));
		return false;
	}

	// 모든 행의 열 수가 동일한지 확인
	const int32 ExpectedCols = WallLayout[0].Num();
	for (int32 Row = 1; Row < WallLayout.Num(); ++Row)
	{
		if (WallLayout[Row].Num() != ExpectedCols)
		{
			OutErrorMessage = FText::FromString(TEXT("행마다 열 수가 일치하지 않습니다."));
			return false;
		}
	}

	// 1. 경계에서 Flood Fill로 외부 영역 마킹
	TSet<FIntPoint> ExteriorCells;
	MarkExteriorCells(WallLayout, ExteriorCells);

	// 2. 외부 영역이 아닌 내부 빈 셀 찾기
	int32 InteriorX = -1, InteriorY = -1;
	bool bFoundInterior = FindInteriorEmptyCell(WallLayout, ExteriorCells, InteriorX, InteriorY);

	if (!bFoundInterior)
	{
		// 내부 빈 공간이 없음 - 모든 빈 셀이 외부(경계에 연결됨)이거나 빈 셀이 없음
		OutErrorMessage = FText::FromString(TEXT("벽으로 둘러싸인 내부 공간이 없습니다. 벽이 완전히 닫힌 형태로 플레이 영역을 만들어주세요."));
		return false;
	}

	OutErrorMessage = FText::FromString(TEXT("유효한 레이아웃입니다."));
	return true;
}

bool FWjWorldWallLayoutConverter::SaveWallLayoutToFile(
	const TArray<TArray<int32>>& WallLayout,
	const FString& FilePath)
{
	FString CSVContent = WallLayoutToCSVString(WallLayout);

	if (!FFileHelper::SaveStringToFile(CSVContent, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("WallLayoutConverter: Failed to save file to %s"), *FilePath);
		return false;
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("WallLayoutConverter: Saved wall layout to %s"), *FilePath);
	return true;
}

void FWjWorldWallLayoutConverter::MarkExteriorCells(
	const TArray<TArray<int32>>& WallLayout,
	TSet<FIntPoint>& OutExteriorCells)
{
	OutExteriorCells.Empty();

	const int32 Rows = WallLayout.Num();
	if (Rows == 0) return;

	const int32 Cols = WallLayout[0].Num();
	if (Cols == 0) return;

	const TArray<FIntPoint> Directions = {
		{ 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
		{ 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 }
	};

	TQueue<FIntPoint> Queue;

	// 경계(가장자리)에 있는 모든 빈 셀을 시작점으로 추가
	// 상단/하단 행
	for (int32 X = 0; X < Cols; ++X)
	{
		if (WallLayout[0][X] == WallCell_Empty)
		{
			FIntPoint Pt(X, 0);
			if (!OutExteriorCells.Contains(Pt))
			{
				OutExteriorCells.Add(Pt);
				Queue.Enqueue(Pt);
			}
		}
		if (WallLayout[Rows - 1][X] == WallCell_Empty)
		{
			FIntPoint Pt(X, Rows - 1);
			if (!OutExteriorCells.Contains(Pt))
			{
				OutExteriorCells.Add(Pt);
				Queue.Enqueue(Pt);
			}
		}
	}

	// 좌측/우측 열
	for (int32 Y = 0; Y < Rows; ++Y)
	{
		if (WallLayout[Y][0] == WallCell_Empty)
		{
			FIntPoint Pt(0, Y);
			if (!OutExteriorCells.Contains(Pt))
			{
				OutExteriorCells.Add(Pt);
				Queue.Enqueue(Pt);
			}
		}
		if (WallLayout[Y][Cols - 1] == WallCell_Empty)
		{
			FIntPoint Pt(Cols - 1, Y);
			if (!OutExteriorCells.Contains(Pt))
			{
				OutExteriorCells.Add(Pt);
				Queue.Enqueue(Pt);
			}
		}
	}

	// Flood Fill: 경계에서 연결된 모든 빈 셀을 외부로 마킹
	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		for (const FIntPoint& Dir : Directions)
		{
			const int32 NX = Current.X + Dir.X;
			const int32 NY = Current.Y + Dir.Y;

			// 범위 체크
			if (NX < 0 || NX >= Cols || NY < 0 || NY >= Rows)
				continue;

			// 벽이면 통과 불가
			if (WallLayout[NY][NX] >= WallCell_Wall)
				continue;

			FIntPoint Next(NX, NY);
			if (!OutExteriorCells.Contains(Next))
			{
				OutExteriorCells.Add(Next);
				Queue.Enqueue(Next);
			}
		}
	}
}

bool FWjWorldWallLayoutConverter::FindInteriorEmptyCell(
	const TArray<TArray<int32>>& WallLayout,
	const TSet<FIntPoint>& ExteriorCells,
	int32& OutX,
	int32& OutY)
{
	for (int32 Y = 0; Y < WallLayout.Num(); ++Y)
	{
		for (int32 X = 0; X < WallLayout[Y].Num(); ++X)
		{
			// 빈 셀이면서 외부 영역이 아닌 경우 = 내부 영역
			if (WallLayout[Y][X] == WallCell_Empty && !ExteriorCells.Contains(FIntPoint(X, Y)))
			{
				OutX = X;
				OutY = Y;
				return true;
			}
		}
	}

	return false;
}
