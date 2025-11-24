// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestSubsystem.generated.h"

class UQuest;
class PacketData;

UCLASS()
class WJWORLD_API UQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	//서버로부터 정해진 시점에 패킷을 받아 퀘스트 데이터를 로드한다.
	void ProcessQuestLoadNtf(TSharedRef<PacketData> Packet);

private:
	TObjectPtr<UQuest> Quest;
};
