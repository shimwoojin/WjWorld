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
    FString FilePath = WallLayoutFilePath.FilePath;

    // 에디터에서 설정한 절대 경로를 Content 상대 경로로 변환
    FString ContentRelativePath;
    int32 ContentIndex = FilePath.Find(TEXT("/Content/"), ESearchCase::IgnoreCase);
    if (ContentIndex == INDEX_NONE)
    {
        ContentIndex = FilePath.Find(TEXT("\\Content\\"), ESearchCase::IgnoreCase);
    }

    if (ContentIndex != INDEX_NONE)
    {
        // "/Content/" 또는 "\\Content\\" 이후의 상대 경로 추출
        ContentRelativePath = FilePath.RightChop(ContentIndex + 9);
        ContentRelativePath = ContentRelativePath.Replace(TEXT("\\"), TEXT("/"));
    }

    // 1차 시도: Content 디렉토리 기준 경로
    FString ResolvedPath = FPaths::ProjectContentDir() / ContentRelativePath;

    if (!FFileHelper::LoadFileToString(FileContent, *ResolvedPath))
    {
        // 2차 시도: 원본 절대 경로 (에디터에서만 동작)
        if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
        {
            UE_LOG(LogWjWorld, Error, TEXT("Failed to load file. Tried paths:\n  1) %s\n  2) %s"), *ResolvedPath, *FilePath);
            return false;
        }
    }

    UE_LOG(LogWjWorld, Log, TEXT("Successfully loaded wall layout from: %s"), *ResolvedPath);

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

int32 UWjWorldWallDescriptionDataAsset::ScanUserWallLayouts(TArray<FWjWorldWallDescription>& OutUserDescriptions) const
{
    OutUserDescriptions.Empty();

    FString UserLayoutDir = GetUserWallLayoutDirectory();

    // 디렉토리 존재 확인
    IFileManager& FileManager = IFileManager::Get();
    if (!FileManager.DirectoryExists(*UserLayoutDir))
    {
        UE_LOG(LogWjWorld, Log, TEXT("ScanUserWallLayouts: User layout directory does not exist: %s"), *UserLayoutDir);
        return 0;
    }

    // CSV 파일 검색
    TArray<FString> FoundFiles;
    FileManager.FindFiles(FoundFiles, *UserLayoutDir, TEXT("*.csv"));

    for (const FString& FileName : FoundFiles)
    {
        FString FileNameNoExt = FPaths::GetBaseFilename(FileName);
        FString FullPath = UserLayoutDir / FileName;

        FWjWorldWallDescription UserDesc;
        UserDesc.WallName = FString::Printf(TEXT("User_%s"), *FileNameNoExt);
        UserDesc.WallLayoutFilePath.FilePath = FullPath;
        UserDesc.BrickSize = FVector(100.0, 100.0, 100.0);  // 기본값
        UserDesc.CenterOffset = FVector::ZeroVector;

        OutUserDescriptions.Add(UserDesc);
        UE_LOG(LogWjWorld, Log, TEXT("ScanUserWallLayouts: Found user layout '%s'"), *UserDesc.WallName);
    }

    return OutUserDescriptions.Num();
}

TArray<FString> UWjWorldWallDescriptionDataAsset::GetAllWallNames() const
{
    TArray<FString> AllNames;

    // 내장 레이아웃
    for (const FWjWorldWallDescription& Desc : WallDescriptions)
    {
        AllNames.Add(Desc.WallName);
    }

    // 유저 레이아웃
    if (!bUserDescriptionsCached)
    {
        ScanUserWallLayouts(CachedUserDescriptions);
        bUserDescriptionsCached = true;
    }

    for (const FWjWorldWallDescription& UserDesc : CachedUserDescriptions)
    {
        AllNames.Add(UserDesc.WallName);
    }

    return AllNames;
}

bool UWjWorldWallDescriptionDataAsset::GetWallDescriptionByNameIncludingUser(const FString& Name, FWjWorldWallDescription& OutDescription) const
{
    // 내장 레이아웃 먼저 검색
    if (GetWallDescriptionByName(Name, OutDescription))
    {
        return true;
    }

    // 유저 레이아웃 검색
    if (!bUserDescriptionsCached)
    {
        ScanUserWallLayouts(CachedUserDescriptions);
        bUserDescriptionsCached = true;
    }

    for (const FWjWorldWallDescription& UserDesc : CachedUserDescriptions)
    {
        if (UserDesc.WallName == Name)
        {
            OutDescription = UserDesc;
            return true;
        }
    }

    return false;
}

FString UWjWorldWallDescriptionDataAsset::GenerateRandomWallNameIncludingUser() const
{
    TArray<FString> AllNames = GetAllWallNames();

    if (AllNames.Num() == 0)
    {
        UE_LOG(LogWjWorld, Error, TEXT("No wall descriptions available (built-in or user)"));
        return FString();
    }

    int32 RandomIndex = FMath::RandRange(0, AllNames.Num() - 1);
    return AllNames[RandomIndex];
}
