// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Quest/QuestInstance.h"
#include "QuestState.h"

TObjectPtr<UQuestInstance> UQuestInstance::FillFromQuestInfo(const ActiveQuestInfo& InQuestInfo)
{
	Update(InQuestInfo);
	return this;
}

EQuestState UQuestInstance::Update(const ActiveQuestInfo& InQuestInfo)
{
	QuestInfo = InQuestInfo;
	return Update();
}

EQuestState UQuestInstance::Update()
{
	QuestState::StateMap[QuestState]->Enter(this);
	return QuestState;
}

bool UQuestInstance::CanAutoCompleted()
{
	return true;
}
