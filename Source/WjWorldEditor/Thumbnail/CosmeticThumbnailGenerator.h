// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UWjWorldCosmeticCatalogDataAsset;

/**
 * 코스메틱 카탈로그 아이템의 메시 썸네일을 UTexture2D 에셋으로 일괄 생성
 * 에디터 콘솔 명령: Cosmetic_GenerateIcons
 */
class FCosmeticThumbnailGenerator
{
public:
	/** 코스메틱 카탈로그 전체 아이콘 생성 */
	static void GenerateAllIcons();

	/** 특정 카탈로그의 아이콘 생성 */
	static int32 GenerateIconsForCatalog(UWjWorldCosmeticCatalogDataAsset* Catalog);

private:
	/** 메시 에셋의 에디터 썸네일을 UTexture2D로 저장 */
	static UTexture2D* RenderAndSaveThumbnail(UObject* MeshAsset, const FString& SavePath);
};
