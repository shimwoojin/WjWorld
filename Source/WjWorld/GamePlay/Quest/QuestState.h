// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UQuestInstance;

enum class EQuestState
{
	None,
	InProgress,
	DoneButNotEnded,
	Ended
};

class WJWORLD_API QuestState
{
public:
	static TMap<EQuestState, QuestState*> StateMap;

public:
    virtual ~QuestState() = default;

    // 1. 상태 진입
    virtual void Enter(UQuestInstance* Quest) = 0;

private:
    // 2. 상태 종료
    virtual void Exit(UQuestInstance* Quest) = 0;

    // 3. 이벤트/액션 처리
    virtual void OnQuestAction(UQuestInstance* Quest) = 0;

    // 4. 상태 전이 처리 또는 검증
    virtual bool CanProgress(UQuestInstance* Quest) = 0;
};

class WJWORLD_API QuestStateNone : public QuestState
{
public:
    virtual void Enter(UQuestInstance* Quest);
    virtual void Exit(UQuestInstance* Quest);
    virtual void OnQuestAction(UQuestInstance* Quest);
    virtual bool CanProgress(UQuestInstance* Quest);
};

class WJWORLD_API QuestStateInProgress : public QuestState
{
public:
    virtual void Enter(UQuestInstance* Quest);
    virtual void Exit(UQuestInstance* Quest);
    virtual void OnQuestAction(UQuestInstance* Quest);
    virtual bool CanProgress(UQuestInstance* Quest);
};

class WJWORLD_API QuestStateDoneButNotEnded : public QuestState
{
public:
    virtual void Enter(UQuestInstance* Quest);
    virtual void Exit(UQuestInstance* Quest);
    virtual void OnQuestAction(UQuestInstance* Quest);
    virtual bool CanProgress(UQuestInstance* Quest);
};

class WJWORLD_API QuestStateEnded : public QuestState
{
public:
    virtual void Enter(UQuestInstance* Quest);
    virtual void Exit(UQuestInstance* Quest);
    virtual void OnQuestAction(UQuestInstance* Quest);
    virtual bool CanProgress(UQuestInstance* Quest);
};
