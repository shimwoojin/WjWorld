// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Quest/QuestState.h"
#include "QuestInstance.h"

TMap<EQuestState, QuestState*> QuestState::StateMap =
{
	{ EQuestState::None, new QuestStateNone() },
	{ EQuestState::InProgress, new QuestStateInProgress() },
	{ EQuestState::DoneButNotEnded, new QuestStateDoneButNotEnded() },
	{ EQuestState::Ended, new QuestStateEnded() },
};

void QuestStateNone::Enter(UQuestInstance* Quest)
{
}

void QuestStateNone::Exit(UQuestInstance* Quest)
{
}

void QuestStateNone::OnQuestAction(UQuestInstance* Quest)
{
}

bool QuestStateNone::CanProgress(UQuestInstance* Quest)
{
	return false;
}

void QuestStateInProgress::Enter(UQuestInstance* Quest)
{
	bool bCanProgress = CanProgress(Quest);
	if (bCanProgress)
	{
		OnQuestAction(Quest);
	}

	Exit(Quest);
}

void QuestStateInProgress::Exit(UQuestInstance* Quest)
{
	if (Quest && Quest->CanAutoCompleted())
	{
		Quest->Update();
	}
}

void QuestStateInProgress::OnQuestAction(UQuestInstance* Quest)
{
}

bool QuestStateInProgress::CanProgress(UQuestInstance* Quest)
{
	return false;
}

void QuestStateDoneButNotEnded::Enter(UQuestInstance* Quest)
{
}

void QuestStateDoneButNotEnded::Exit(UQuestInstance* Quest)
{
}

void QuestStateDoneButNotEnded::OnQuestAction(UQuestInstance* Quest)
{
}

bool QuestStateDoneButNotEnded::CanProgress(UQuestInstance* Quest)
{
	return false;
}

void QuestStateEnded::Enter(UQuestInstance* Quest)
{
}

void QuestStateEnded::Exit(UQuestInstance* Quest)
{
}

void QuestStateEnded::OnQuestAction(UQuestInstance* Quest)
{
}

bool QuestStateEnded::CanProgress(UQuestInstance* Quest)
{
	return false;
}
