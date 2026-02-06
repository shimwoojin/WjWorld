// Fill out your copyright notice in the Description page of Project Settings.

#include "Setting/WjWorldDeveloperSettings.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"

FString UWjWorldDeveloperSettings::GetLobbyMapPath() const
{
	// ServerTravel/OpenLevel에는 패키지 경로만 필요 (에셋 이름 제외)
	return LobbyMapPath.GetLongPackageName();
}

FString UWjWorldDeveloperSettings::GetWaitingRoomOpenLevelURL() const
{
	// ServerTravel/OpenLevel에는 패키지 경로만 필요 (에셋 이름 제외)
	FString MapPath = LobbyMapPath.GetLongPackageName();
	FString GameModeClassPath = WaitingRoomGameModeClass.IsNull()
		? TEXT("")
		: WaitingRoomGameModeClass.ToString();

	if (GameModeClassPath.IsEmpty())
	{
		return MapPath;
	}

	// OpenLevel URL: MapPath?game=GameModeClass?Listen
	return FString::Printf(TEXT("%s?game=%s?Listen"), *MapPath, *GameModeClassPath);
}

FString UWjWorldDeveloperSettings::GetWaitingRoomServerTravelURL() const
{
	// ServerTravel에는 패키지 경로만 필요 (에셋 이름 제외)
	FString MapPath = LobbyMapPath.GetLongPackageName();
	FString GameModeClassPath = WaitingRoomGameModeClass.IsNull()
		? TEXT("")
		: WaitingRoomGameModeClass.ToString();

	if (GameModeClassPath.IsEmpty())
	{
		return MapPath;
	}

	// ServerTravel URL: MapPath?game=GameModeClass
	return FString::Printf(TEXT("%s?game=%s"), *MapPath, *GameModeClassPath);
}

FString UWjWorldDeveloperSettings::GetPlayServerTravelURL(const FString& LevelPath, const FString& GameModeId) const
{
	FString GameModeClassPath = PlayGameModeClass.IsNull()
		? TEXT("")
		: PlayGameModeClass.ToString();

	if (GameModeClassPath.IsEmpty())
	{
		return LevelPath;
	}

	// ServerTravel URL: LevelPath?game=GameModeClass?GameModeId=xxx
	return FString::Printf(TEXT("%s?game=%s?GameModeId=%s"), *LevelPath, *GameModeClassPath, *GameModeId);
}

UWjWorldPlaceableObjectDataAsset* UWjWorldDeveloperSettings::GetPlaceableCatalogForContext(EPlacementContext Context) const
{
	TSoftObjectPtr<UWjWorldPlaceableObjectDataAsset> CatalogRef;

	switch (Context)
	{
	case EPlacementContext::Lobby:
		CatalogRef = LobbyPlaceableCatalog;
		break;
	case EPlacementContext::ApproachingWall:
		CatalogRef = ApproachingWallPlaceableCatalog;
		break;
	case EPlacementContext::JumpMap:
		CatalogRef = JumpMapPlaceableCatalog;
		break;
	default:
		CatalogRef = PlaceableObjectCatalog;
		break;
	}

	// 컨텍스트별 카탈로그가 없으면 레거시 폴백
	if (CatalogRef.IsNull())
	{
		CatalogRef = PlaceableObjectCatalog;
	}

	if (CatalogRef.IsNull())
	{
		return nullptr;
	}

	return CatalogRef.LoadSynchronous();
}

FString UWjWorldDeveloperSettings::GetEditorMapOpenLevelURL(EPlacementContext Context) const
{
	FSoftObjectPath MapPath;
	TSoftClassPtr<AGameModeBase> GameModeClass;

	switch (Context)
	{
	case EPlacementContext::ApproachingWall:
		MapPath = AWEditorMapPath;
		GameModeClass = AWEditorGameModeClass;
		break;
	case EPlacementContext::JumpMap:
		MapPath = JumpMapEditorMapPath;
		GameModeClass = JumpMapEditorGameModeClass;
		break;
	default:
		// Lobby는 에디터 맵이 없음 (현재 맵에서 배치)
		return FString();
	}

	if (!MapPath.IsValid())
	{
		return FString();
	}

	FString MapPathStr = MapPath.GetLongPackageName();
	FString GameModeClassPath = GameModeClass.IsNull() ? TEXT("") : GameModeClass.ToString();

	if (GameModeClassPath.IsEmpty())
	{
		return MapPathStr;
	}

	// OpenLevel URL: MapPath?game=GameModeClass
	return FString::Printf(TEXT("%s?game=%s"), *MapPathStr, *GameModeClassPath);
}

bool UWjWorldDeveloperSettings::HasEditorMapForContext(EPlacementContext Context) const
{
	switch (Context)
	{
	case EPlacementContext::ApproachingWall:
		return AWEditorMapPath.IsValid();
	case EPlacementContext::JumpMap:
		return JumpMapEditorMapPath.IsValid();
	case EPlacementContext::Lobby:
		return true; // 로비는 항상 사용 가능 (현재 맵)
	default:
		return false;
	}
}
