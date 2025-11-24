// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Quest/QuestFactory.h"
#include "Network/PacketDataQuest.h"
#include "QuestInstance.h"

TObjectPtr<UQuestInstance> UQuestFactory::CreateQuestInstance(const ActiveQuestInfo& InQuestInfo)
{
	TObjectPtr<UQuestInstance> Temp = NewObject<UQuestInstance>(this);
	return Temp->FillFromQuestInfo(InQuestInfo);
}
