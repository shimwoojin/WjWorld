// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestState.h"
#include "Network/PacketDataQuest.h"
#include "QuestInstance.generated.h"

UCLASS()
class WJWORLD_API UQuestInstance : public UObject
{
	GENERATED_BODY()
	
public:
	TObjectPtr<UQuestInstance> FillFromQuestInfo(const ActiveQuestInfo& InQuestInfo);
	EQuestState Update(const ActiveQuestInfo& InQuestInfo);
	EQuestState Update();
	bool CanAutoCompleted();

private:
	EQuestState QuestState;
	ActiveQuestInfo QuestInfo;
};
