// Fill out your copyright notice in the Description page of Project Settings.

#include "Setting/WjWorldDeveloperSettings.h"

FString UWjWorldDeveloperSettings::GetLobbyMapPath() const
{
	return LobbyMapPath.GetAssetPathString();
}

FString UWjWorldDeveloperSettings::GetWaitingRoomOpenLevelURL() const
{
	FString MapPath = LobbyMapPath.GetAssetPathString();
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
	FString MapPath = LobbyMapPath.GetAssetPathString();
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
