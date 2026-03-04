// Fill out your copyright notice in the Description page of Project Settings.

#include "Thumbnail/CosmeticThumbnailGenerator.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "Setting/WjWorldDeveloperSettings.h"

#include "ObjectTools.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Engine/Texture2D.h"
#include "Misc/PackageName.h"
#include "HAL/PlatformFileManager.h"

// 콘솔 명령 등록
static FAutoConsoleCommand CmdCosmeticGenerateIcons(
	TEXT("Cosmetic_GenerateIcons"),
	TEXT("코스메틱 카탈로그 아이템의 메시 썸네일을 UTexture2D 에셋으로 일괄 생성"),
	FConsoleCommandDelegate::CreateStatic(&FCosmeticThumbnailGenerator::GenerateAllIcons)
);

void FCosmeticThumbnailGenerator::GenerateAllIcons()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("CosmeticThumbnailGenerator: DeveloperSettings not found"));
		return;
	}

	UWjWorldCosmeticCatalogDataAsset* Catalog = Settings->CosmeticCatalog.LoadSynchronous();
	if (!Catalog)
	{
		UE_LOG(LogTemp, Error, TEXT("CosmeticThumbnailGenerator: CosmeticCatalog not found"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("CosmeticThumbnailGenerator: Processing catalog '%s' (%d items)"),
		*Catalog->GetName(), Catalog->Items.Num());

	int32 Count = GenerateIconsForCatalog(Catalog);

	UE_LOG(LogTemp, Log, TEXT("CosmeticThumbnailGenerator: Complete — %d icons generated"), Count);
}

int32 FCosmeticThumbnailGenerator::GenerateIconsForCatalog(UWjWorldCosmeticCatalogDataAsset* Catalog)
{
	if (!Catalog)
	{
		return 0;
	}

	int32 GeneratedCount = 0;

	for (FCosmeticItemDefinition& Def : Catalog->Items)
	{
		if (!Def.IsValid())
		{
			continue;
		}

		// 메시 로드 (StaticMesh 우선, 없으면 SkeletalMesh)
		UObject* MeshAsset = nullptr;
		if (!Def.StaticMesh.IsNull())
		{
			MeshAsset = Def.StaticMesh.LoadSynchronous();
		}
		if (!MeshAsset && !Def.SkeletalMesh.IsNull())
		{
			MeshAsset = Def.SkeletalMesh.LoadSynchronous();
		}

		if (!MeshAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("  [SKIP] %s — no mesh"), *Def.ItemId.ToString());
			continue;
		}

		// 저장 경로: Content/UI/Textures/Cosmetic/T_Cosmetic_{ItemId}
		FString AssetName = FString::Printf(TEXT("T_Cosmetic_%s"), *Def.ItemId.ToString());
		FString SavePath = FString::Printf(TEXT("/Game/UI/Textures/Cosmetic/%s"), *AssetName);

		UTexture2D* GeneratedTexture = RenderAndSaveThumbnail(MeshAsset, SavePath);
		if (GeneratedTexture)
		{
			Def.Icon = GeneratedTexture;
			GeneratedCount++;
			UE_LOG(LogTemp, Log, TEXT("  [OK] %s → %s"), *Def.ItemId.ToString(), *SavePath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  [FAIL] %s — thumbnail generation failed"), *Def.ItemId.ToString());
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

UTexture2D* FCosmeticThumbnailGenerator::RenderAndSaveThumbnail(UObject* MeshAsset, const FString& SavePath)
{
	if (!MeshAsset)
	{
		return nullptr;
	}

	// 에디터 썸네일 렌더링
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

	// 알파 채널을 255(불투명)로 강제 설정 — BGRA8 포맷
	for (int32 i = 3; i < ImageData.Num(); i += 4)
	{
		ImageData[i] = 255;
	}

	// 패키지 생성
	FString PackageName = SavePath;
	FString AssetName = FPackageName::GetShortName(PackageName);

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

	uint8* MipData = NewTexture->Source.LockMip(0);
	if (MipData)
	{
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
		FAssetRegistryModule::AssetCreated(NewTexture);
		return NewTexture;
	}

	return nullptr;
}
