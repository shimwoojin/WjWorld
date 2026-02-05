// Fill out your copyright notice in the Description page of Project Settings.

#include "Stats/WjWorldStatTypes.h"

const TArray<FMinigameStatDescriptor>& GetAllMinigameDescriptors()
{
	static TArray<FMinigameStatDescriptor> Descriptors;

	if (Descriptors.Num() == 0)
	{
		// Approaching Wall
		{
			FMinigameStatDescriptor Desc;
			Desc.MinigameName = NSLOCTEXT("WjWorldStats", "AW_Name", "Approaching Wall");
			Desc.GameModeId = TEXT("ApproachingWall");
			Desc.Stats = {
				{ WjWorldStats::ApproachingWall::GamesPlayed, NSLOCTEXT("WjWorldStats", "AW_GamesPlayed", "Games Played") },
				{ WjWorldStats::ApproachingWall::Wins,        NSLOCTEXT("WjWorldStats", "AW_Wins", "Wins") },
				{ WjWorldStats::ApproachingWall::Losses,      NSLOCTEXT("WjWorldStats", "AW_Losses", "Losses") },
				{ WjWorldStats::ApproachingWall::Kills,       NSLOCTEXT("WjWorldStats", "AW_Kills", "Kills") },
			};
			Descriptors.Add(MoveTemp(Desc));
		}

		// Sumo
		{
			FMinigameStatDescriptor Desc;
			Desc.MinigameName = NSLOCTEXT("WjWorldStats", "Sumo_Name", "Sumo Knockoff");
			Desc.GameModeId = TEXT("Sumo");
			Desc.Stats = {
				{ WjWorldStats::Sumo::GamesPlayed, NSLOCTEXT("WjWorldStats", "Sumo_GamesPlayed", "Games Played") },
				{ WjWorldStats::Sumo::Wins,        NSLOCTEXT("WjWorldStats", "Sumo_Wins", "Wins") },
				{ WjWorldStats::Sumo::Losses,      NSLOCTEXT("WjWorldStats", "Sumo_Losses", "Losses") },
				{ WjWorldStats::Sumo::Kills,       NSLOCTEXT("WjWorldStats", "Sumo_Kills", "Kills") },
			};
			Descriptors.Add(MoveTemp(Desc));
		}

		// 향후 미니게임 추가 시 여기에 디스크립터 추가
	}

	return Descriptors;
}
