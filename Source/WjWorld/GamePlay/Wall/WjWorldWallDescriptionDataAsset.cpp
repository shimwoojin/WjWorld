// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Wall/WjWorldWallDescriptionDataAsset.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "WjWorldLogCategories.h"

bool FWjWorldWallDescription::LoadWallLayoutFromFile()
{
    WallLayout.Empty();

    if (WallLayoutFilePath.FilePath.IsEmpty())
    {
        UE_LOG(LogWjWorld, Warning, TEXT("WallLayoutFilePath is empty"));
        return false;
    }

    FString FileContent;
    const FString AbsolutePath = WallLayoutFilePath.FilePath;

    if (!FFileHelper::LoadFileToString(FileContent, *AbsolutePath))
    {
        UE_LOG(LogWjWorld, Error, TEXT("Failed to load file: %s"), *AbsolutePath);
        return false;
    }

    bool bParseResult = ParseWallLayout(FileContent);
    if(!bParseResult)
    {
        bIsLayoutValid = false;
        return false;
	}

    if(RowNum > 0 && ColumnNum > 0)
    {
		int32 StartX = -1, StartY = -1;
        bIsLayoutValid = (FindStartingEmptyCell(StartX, StartY) && IsAreaEnclosedByWalls(StartX, StartY));
    }
    else
    {
        bIsLayoutValid = false;
	}

    return bIsLayoutValid;
}

void FWjWorldWallDescription::GetRandomSafeZones(TArray<FIntPoint>& OutSafeZones, int32 MaxCount) const
{
    OutSafeZones.Reset();

    if (SafeZones.Num() == 0 || MaxCount <= 0)
    {
        return;
    }

    // 원본 보호용 복사
    TArray<FIntPoint> ShuffledZones = SafeZones;

    // 랜덤 셔플
    for (int32 i = ShuffledZones.Num() - 1; i > 0; --i)
    {
        const int32 RandIndex = FMath::RandRange(0, i);
        ShuffledZones.Swap(i, RandIndex);
    }

    const int32 CountToTake = FMath::Min(MaxCount, ShuffledZones.Num());

    for (int32 i = 0; i < CountToTake; ++i)
    {
        OutSafeZones.Add(ShuffledZones[i]);
    }
}

bool FWjWorldWallDescription::ParseWallLayout(const FString& FileContent)
{
    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

	int32 BeforeColumnNum = -1;

    for (const FString& Line : Lines)
    {
        if (Line.IsEmpty())
            continue;

        TArray<FString> Values;
        Line.ParseIntoArray(Values, TEXT(","), true);

        TArray<int32> Row;
        for (const FString& Value : Values)
        {
            Row.Add(FCString::Atoi(*Value));
        }

        if (BeforeColumnNum == -1)
        {
            BeforeColumnNum = Row.Num();
			ColumnNum = BeforeColumnNum;
        }
        else if (BeforeColumnNum != Row.Num())
        {
            UE_LOG(LogWjWorld, Error, TEXT("Inconsistent column count in wall layout file. Expected: %d, Found: %d"), BeforeColumnNum, Row.Num());
            bSizeExactMatch = false;
            return false;
        }

        WallLayout.Add(Row);
    }

    RowNum = WallLayout.Num();
    bSizeExactMatch = true;

    UE_LOG(LogWjWorld, Log, TEXT("Wall layout loaded: %d rows, %d columns"), RowNum, ColumnNum);
    return true;
}

bool FWjWorldWallDescription::FindStartingEmptyCell(int32& OutX, int32& OutY)
{
    for(int32 Y = 0; Y < WallLayout.Num(); ++Y)
    {
        for(int32 X = 0; X < WallLayout[Y].Num(); ++X)
        {
            if(WallLayout[Y][X] == WallCell_Empty)
            {
                OutX = X;
                OutY = Y;
                return true;
            }
        }
	}

    return false;
}

bool FWjWorldWallDescription::IsAreaEnclosedByWalls(int32 StartX, int32 StartY)
{
    const int32 Rows = WallLayout.Num();
    if (Rows == 0) return false;

    const int32 Cols = WallLayout[0].Num();

    // 시작 위치 유효성
    if (StartX < 0 || StartX >= Cols || StartY < 0 || StartY >= Rows)
        return false;

    // 시작 지점이 벽이면 검사 불필요
    if (WallLayout[StartY][StartX] >= 1)
        return true;

    TQueue<FIntPoint> Queue;
    TSet<FIntPoint> Visited;

    Queue.Enqueue(FIntPoint(StartX, StartY));
    Visited.Add(FIntPoint(StartX, StartY));

    const TArray<FIntPoint> Directions = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
    };

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        for (const FIntPoint& Dir : Directions)
        {
            const int32 NX = Current.X + Dir.X;
            const int32 NY = Current.Y + Dir.Y;

            if (NX < 0 || NX >= Cols || NY < 0 || NY >= Rows)
            {
                return false;
            }

            // 벽(1 이상)이면 통과 불가
            if (WallLayout[NY][NX] >= 1)
                continue;

            FIntPoint Next(NX, NY);
            if (!Visited.Contains(Next))
            {
                Visited.Add(Next);
                Queue.Enqueue(Next);
            }
        }
    }

    SafeZones.Empty();
    SafeZones.Append(Visited.Array());  

    SafeZonesSet = Visited;

    // 끝까지 못 나갔으면 완전히 둘러싸임
    return true;
}
