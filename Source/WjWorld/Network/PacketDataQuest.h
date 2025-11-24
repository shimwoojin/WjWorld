// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketData.h"

struct WJWORLD_API ActiveQuestInfo
{
	tid QuestTID;
};

class WJWORLD_API PacketQuestLoadNtf : public PacketData
{
public:
	TArray<tid> InactiveQuestTIDs;
	TArray<tid> CompletedQuestTIDs;
	TArray<ActiveQuestInfo> ActiveQuests;
};
