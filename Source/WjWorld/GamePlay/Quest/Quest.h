// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WjTypes.h"
#include "Quest.generated.h"

class PacketQuestLoadNtf;
class UQuestInstance;
class UQuestFactory;

struct ActiveQuestInfo;

DECLARE_DELEGATE_OneParam(FQuestActiveSignature, TObjectPtr<UQuestInstance> /*QuestInstance*/);
DECLARE_DELEGATE_OneParam(FQuestUpdateSignature, TObjectPtr<UQuestInstance> /*QuestInstance*/);
DECLARE_DELEGATE_OneParam(FQuestCompleteAlertSignature, TObjectPtr<UQuestInstance> /*QuestInstance*/);
DECLARE_DELEGATE_OneParam(FQuestCompleteSignature, TObjectPtr<UQuestInstance> /*QuestInstance*/);

UCLASS()
class WJWORLD_API UQuest : public UObject
{
	GENERATED_BODY()
	
public:
	bool LoadQuestDataFromServer(TSharedRef<PacketQuestLoadNtf> InData);
	void Update(const ActiveQuestInfo& InQuestInfo);

public:
	FQuestActiveSignature OnQuestActive;
	FQuestUpdateSignature OnQuestUpdate;
	FQuestCompleteAlertSignature OnQuestCompleteAlert;
	FQuestCompleteSignature OnQuestComplete;

private:
	TObjectPtr<UQuestFactory> QuestFactory;

	TSet<tid> InactiveQuestTIDs;
	TSet<tid> CompletedQuestTIDs;
	TMap<tid, TObjectPtr<UQuestInstance>> ActiveQuestInstances;
};
