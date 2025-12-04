// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Quest/Quest.h"
#include "Network/PacketDataQuest.h"
#include "QuestFactory.h"
#include "QuestInstance.h"
#include "WjWorldLogCategories.h"

bool UQuest::LoadQuestDataFromServer(TSharedRef<PacketQuestLoadNtf> InData)
{
	check(QuestFactory);

	for (tid TID : InData->InactiveQuestTIDs)
	{
		InactiveQuestTIDs.Add(TID);
	}

	for (tid TID : InData->CompletedQuestTIDs)
	{
		CompletedQuestTIDs.Add(TID);
	}

	for (const auto& Quest : InData->ActiveQuests)
	{
		TObjectPtr<UQuestInstance> QuestInstance = QuestFactory->CreateQuestInstance(Quest);
		ActiveQuestInstances.Add(Quest.QuestTID, QuestInstance);
	}

	return true;
}

void UQuest::Update(const ActiveQuestInfo& InQuestInfo)
{
	check(QuestFactory);

	tid TID = InQuestInfo.QuestTID;

	if(InactiveQuestTIDs.Contains(TID))
	{
		InactiveQuestTIDs.Remove(TID);
		TObjectPtr<UQuestInstance> QuestInstance = QuestFactory->CreateQuestInstance(InQuestInfo);
		ActiveQuestInstances.Add(TID, QuestInstance);

		OnQuestActive.ExecuteIfBound(QuestInstance);
	}
	else if (ActiveQuestInstances.Contains(TID))
	{
		EQuestState QuestState = ActiveQuestInstances[TID]->Update(InQuestInfo);
		switch (QuestState)
		{
		case EQuestState::None:
			break;
		case EQuestState::InProgress:
		{
			OnQuestUpdate.ExecuteIfBound(ActiveQuestInstances[TID]);
			break;
		}
		case EQuestState::DoneButNotEnded:
		{
			OnQuestCompleteAlert.ExecuteIfBound(ActiveQuestInstances[TID]);
			break;
		}
		case EQuestState::Ended:
		{
			ActiveQuestInstances.Remove(TID);
			CompletedQuestTIDs.Add(TID);

			OnQuestComplete.ExecuteIfBound(ActiveQuestInstances[TID]);
			break;
		}
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("Quest Update Error: Quest TID %d not found in Inactive or Active quests."), TID);
	}
}
