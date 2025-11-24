// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestFactory.generated.h"

class UQuestInstance;
struct ActiveQuestInfo;

UCLASS()
class WJWORLD_API UQuestFactory : public UObject
{
	GENERATED_BODY()
	
public:
	TObjectPtr<UQuestInstance> CreateQuestInstance(const ActiveQuestInfo& InQuestInfo);
};
