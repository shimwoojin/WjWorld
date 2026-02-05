// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Quest/QuestSubsystem.h"
#include "Quest.h"
#include "Network/PacketDataQuest.h"

void UQuestSubsystem::ProcessQuestLoadNtf(TSharedRef<PacketData> Packet)
{
	TSharedRef<PacketQuestLoadNtf> QuestLoadNtf = StaticCastSharedRef<PacketQuestLoadNtf>(Packet);
	
	if (Quest && Quest->LoadQuestDataFromServer(QuestLoadNtf))
	{
		//퀘스트 데이터 로드 성공
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UQuestSubsystem::ProcessQuestLoadNtf - 퀘스트 데이터 로드 실패"));
	}
}
