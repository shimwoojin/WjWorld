// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapLayoutDataAsset.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "WjWorldLogCategories.h"

int32 UJumpMapLayoutDataAsset::ScanUserJumpMapLayouts(TArray<FJumpMapLayout>& OutUserLayouts) const
{
	OutUserLayouts.Empty();

	FString UserLayoutDir = GetUserJumpMapLayoutDirectory();

	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*UserLayoutDir))
	{
		UE_LOG(LogWjWorld, Log, TEXT("ScanUserJumpMapLayouts: User layout directory does not exist: %s"), *UserLayoutDir);
		return 0;
	}

	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *UserLayoutDir, TEXT("*.csv"));

	for (const FString& FileName : FoundFiles)
	{
		FString FullPath = UserLayoutDir / FileName;
		FString FileContent;

		if (!FFileHelper::LoadFileToString(FileContent, *FullPath))
		{
			UE_LOG(LogWjWorld, Warning, TEXT("ScanUserJumpMapLayouts: Failed to read file: %s"), *FullPath);
			continue;
		}

		FJumpMapLayout Layout;
		if (ParseLayoutCSV(FileContent, Layout))
		{
			// 레이아웃 이름이 없으면 파일명 사용
			if (Layout.LayoutName.IsEmpty())
			{
				Layout.LayoutName = FString::Printf(TEXT("User_%s"), *FPaths::GetBaseFilename(FileName));
			}
			OutUserLayouts.Add(Layout);
			UE_LOG(LogWjWorld, Log, TEXT("ScanUserJumpMapLayouts: Found user layout '%s'"), *Layout.LayoutName);
		}
	}

	return OutUserLayouts.Num();
}

const FJumpMapLayout* UJumpMapLayoutDataAsset::GetLayoutByName(const FString& Name) const
{
	for (const FJumpMapLayout& Layout : BuiltInLayouts)
	{
		if (Layout.LayoutName == Name)
		{
			return &Layout;
		}
	}
	return nullptr;
}

const FJumpMapLayout* UJumpMapLayoutDataAsset::GetLayoutByNameIncludingUser(const FString& Name) const
{
	// 내장 레이아웃 먼저 검색
	const FJumpMapLayout* BuiltIn = GetLayoutByName(Name);
	if (BuiltIn)
	{
		return BuiltIn;
	}

	// 유저 레이아웃 검색
	if (!bUserLayoutsCached)
	{
		ScanUserJumpMapLayouts(CachedUserLayouts);
		bUserLayoutsCached = true;
	}

	for (const FJumpMapLayout& Layout : CachedUserLayouts)
	{
		if (Layout.LayoutName == Name)
		{
			return &Layout;
		}
	}

	return nullptr;
}

TArray<FString> UJumpMapLayoutDataAsset::GetAllLayoutNames() const
{
	TArray<FString> AllNames;

	for (const FJumpMapLayout& Layout : BuiltInLayouts)
	{
		AllNames.Add(Layout.LayoutName);
	}

	if (!bUserLayoutsCached)
	{
		ScanUserJumpMapLayouts(CachedUserLayouts);
		bUserLayoutsCached = true;
	}

	for (const FJumpMapLayout& Layout : CachedUserLayouts)
	{
		AllNames.Add(Layout.LayoutName);
	}

	return AllNames;
}

FString UJumpMapLayoutDataAsset::GetRandomLayoutName() const
{
	TArray<FString> AllNames = GetAllLayoutNames();

	if (AllNames.Num() == 0)
	{
		UE_LOG(LogWjWorld, Error, TEXT("No JumpMap layouts available (built-in or user)"));
		return FString();
	}

	int32 RandomIndex = FMath::RandRange(0, AllNames.Num() - 1);
	return AllNames[RandomIndex];
}

bool UJumpMapLayoutDataAsset::ParseLayoutCSV(const FString& FileContent, FJumpMapLayout& OutLayout) const
{
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);

	bool bHeaderParsed = false;

	for (const FString& Line : Lines)
	{
		if (Line.IsEmpty()) continue;

		// 메타데이터 라인 (#META:Key:Value)
		if (Line.StartsWith(TEXT("#")))
		{
			if (Line.StartsWith(TEXT("#META:MapName:")))
			{
				OutLayout.LayoutName = Line.RightChop(14); // "#META:MapName:" 길이 = 14
				OutLayout.LayoutName.TrimStartAndEndInline();
			}
			continue;
		}

		// CSV 헤더 행 건너뛰기
		if (!bHeaderParsed && Line.Contains(TEXT("ObjectId")))
		{
			bHeaderParsed = true;
			continue;
		}

		// 데이터 행 파싱: ObjectId,PosX,PosY,PosZ,RotPitch,RotYaw,RotRoll,ScaleX,ScaleY,ScaleZ
		TArray<FString> Values;
		Line.ParseIntoArray(Values, TEXT(","), true);

		if (Values.Num() < 10)
		{
			UE_LOG(LogWjWorld, Warning, TEXT("ParseLayoutCSV: Skipping line with insufficient columns (%d): %s"),
				Values.Num(), *Line);
			continue;
		}

		FJumpMapLayoutEntry Entry;
		Entry.ObjectId = FName(*Values[0].TrimStartAndEnd());

		FVector Location;
		Location.X = FCString::Atof(*Values[1]);
		Location.Y = FCString::Atof(*Values[2]);
		Location.Z = FCString::Atof(*Values[3]);

		FRotator Rotation;
		Rotation.Pitch = FCString::Atof(*Values[4]);
		Rotation.Yaw = FCString::Atof(*Values[5]);
		Rotation.Roll = FCString::Atof(*Values[6]);

		FVector Scale;
		Scale.X = FCString::Atof(*Values[7]);
		Scale.Y = FCString::Atof(*Values[8]);
		Scale.Z = FCString::Atof(*Values[9]);

		Entry.Transform = FTransform(Rotation, Location, Scale);
		OutLayout.Objects.Add(Entry);
	}

	UE_LOG(LogWjWorld, Log, TEXT("ParseLayoutCSV: Parsed %d objects for layout '%s'"),
		OutLayout.Objects.Num(), *OutLayout.LayoutName);
	return OutLayout.Objects.Num() > 0;
}

FString UJumpMapLayoutDataAsset::GetUserJumpMapLayoutDirectory()
{
	return FPaths::ProjectContentDir() / TEXT("JumpMapLayouts") / TEXT("User");
}
