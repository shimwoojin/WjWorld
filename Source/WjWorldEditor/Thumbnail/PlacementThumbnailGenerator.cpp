// Fill out your copyright notice in the Description page of Project Settings.

#include "Thumbnail/PlacementThumbnailGenerator.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "GamePlay/Placement/WjWorldPlacementTypes.h"

#include "ObjectTools.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Engine/Texture2D.h"
#include "Misc/PackageName.h"
#include "HAL/PlatformFileManager.h"

// 콘솔 명령 등록
static FAutoConsoleCommand CmdGenerateIcons(
	TEXT("Placement_GenerateIcons"),
	TEXT("배치 카탈로그 아이템의 메시 썸네일을 UTexture2D 에셋으로 일괄 생성"),
	FConsoleCommandDelegate::CreateStatic(&FPlacementThumbnailGenerator::GenerateAllIcons)
);

void FPlacementThumbnailGenerator::GenerateAllIcons()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("PlacementThumbnailGenerator: DeveloperSettings not found"));
		return;
	}

	int32 TotalGenerated = 0;

	// 모든 컨텍스트 카탈로그 순회
	const EPlacementContext Contexts[] = {
		EPlacementContext::Lobby,
		EPlacementContext::ApproachingWall,
		EPlacementContext::JumpMap
	};

	for (EPlacementContext Context : Contexts)
	{
		UWjWorldPlaceableObjectDataAsset* Catalog = Settings->GetPlaceableCatalogForContext(Context);
		if (Catalog)
		{
			UE_LOG(LogTemp, Log, TEXT("PlacementThumbnailGenerator: Processing catalog '%s' (Context: %s)"),
				*Catalog->GetName(), *GetPlacementContextName(Context));

			int32 Count = GenerateIconsForCatalog(Catalog);
			TotalGenerated += Count;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PlacementThumbnailGenerator: Complete — %d icons generated"), TotalGenerated);
}

int32 FPlacementThumbnailGenerator::GenerateIconsForCatalog(UWjWorldPlaceableObjectDataAsset* Catalog)
{
	if (!Catalog)
	{
		return 0;
	}

	int32 GeneratedCount = 0;

	for (FPlaceableObjectDefinition& Def : Catalog->Objects)
	{
		// Icon이 이미 설정되어 있으면 스킵
		//if (!Def.Icon.IsNull())
		//{
		//	continue;
		//}

		// Mesh 로드
		UObject* MeshAsset = nullptr;
		if (!Def.Mesh.IsNull())
		{
			MeshAsset = Def.Mesh.LoadSynchronous();
		}

		if (!MeshAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("  [SKIP] %s — no mesh"), *Def.ObjectId.ToString());
			continue;
		}

		// 저장 경로: Content/UI/Textures/Placement/T_Place_{ObjectId}
		FString AssetName = FString::Printf(TEXT("T_Place_%s"), *Def.ObjectId.ToString());
		FString SavePath = FString::Printf(TEXT("/Game/UI/Textures/Placement/%s"), *AssetName);

		UTexture2D* GeneratedTexture = RenderAndSaveThumbnail(MeshAsset, SavePath);
		if (GeneratedTexture)
		{
			Def.Icon = GeneratedTexture;
			GeneratedCount++;
			UE_LOG(LogTemp, Log, TEXT("  [OK] %s → %s"), *Def.ObjectId.ToString(), *SavePath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  [FAIL] %s — thumbnail generation failed"), *Def.ObjectId.ToString());
		}
	}

	// 카탈로그 DataAsset 저장 (Icon 참조 반영)
	if (GeneratedCount > 0)
	{
		Catalog->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("  Catalog '%s' marked dirty (%d items updated) — save in editor to persist"),
			*Catalog->GetName(), GeneratedCount);
	}

	return GeneratedCount;
}

UTexture2D* FPlacementThumbnailGenerator::RenderAndSaveThumbnail(UObject* MeshAsset, const FString& SavePath)
{
	if (!MeshAsset)
	{
		return nullptr;
	}

	// 에디터 썸네일 렌더링 (AlwaysFlush로 렌더링 완료 보장)
	FObjectThumbnail Thumbnail;
	ThumbnailTools::RenderThumbnail(MeshAsset, 256, 256, ThumbnailTools::EThumbnailTextureFlushMode::AlwaysFlush, nullptr, &Thumbnail);

	if (Thumbnail.GetImageWidth() == 0 || Thumbnail.GetImageHeight() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("  Thumbnail render returned empty for %s"), *MeshAsset->GetName());
		return nullptr;
	}

	const int32 Width = Thumbnail.GetImageWidth();
	const int32 Height = Thumbnail.GetImageHeight();
	TArray<uint8> ImageData = Thumbnail.GetUncompressedImageData();

	if (ImageData.Num() == 0)
	{
		return nullptr;
	}

	// 알파 채널을 255(불투명)로 강제 설정 — 썸네일 배경이 투명으로 렌더링되는 문제 수정
	// BGRA8 포맷: [B, G, R, A] 4바이트 반복
	for (int32 i = 3; i < ImageData.Num(); i += 4)
	{
		ImageData[i] = 255;
	}

	// 패키지 생성
	FString PackageName = SavePath;
	FString AssetName = FPackageName::GetShortName(PackageName);

	// 기존 에셋이 있으면 재사용
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		return nullptr;
	}

	// UTexture2D 생성
	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *AssetName, RF_Public | RF_Standalone);
	if (!NewTexture)
	{
		return nullptr;
	}

	// 텍스처 초기화
	NewTexture->Source.Init(Width, Height, 1, 1, TSF_BGRA8);

	// 소스 데이터에 썸네일 이미지 복사
	uint8* MipData = NewTexture->Source.LockMip(0);
	if (MipData)
	{
		// FObjectThumbnail은 BGRA8 포맷
		FMemory::Memcpy(MipData, ImageData.GetData(), ImageData.Num());
		NewTexture->Source.UnlockMip(0);
	}

	// 텍스처 설정
	NewTexture->SRGB = true;
	NewTexture->CompressionSettings = TC_Default;
	NewTexture->MipGenSettings = TMGS_NoMipmaps;
	NewTexture->NeverStream = true;
	NewTexture->UpdateResource();

	// 에셋 저장
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	// 디렉토리 생성
	FString Directory = FPaths::GetPath(PackageFileName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	bool bSaved = UPackage::SavePackage(Package, NewTexture, *PackageFileName, SaveArgs);

	if (bSaved)
	{
		// 에셋 레지스트리에 등록
		FAssetRegistryModule::AssetCreated(NewTexture);
		return NewTexture;
	}

	return nullptr;
}
