// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameData/SumoGameDataComponent.h"
#include "Net/UnrealNetwork.h"

void USumoGameDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USumoGameDataComponent, AlivePlayerCount);
	DOREPLIFETIME(USumoGameDataComponent, TotalPlayerCount);
	DOREPLIFETIME(USumoGameDataComponent, LastKillFeedText);
	DOREPLIFETIME(USumoGameDataComponent, KillFeedCounter);
	DOREPLIFETIME(USumoGameDataComponent, CurrentRound);
	DOREPLIFETIME(USumoGameDataComponent, MaxRounds);
	DOREPLIFETIME(USumoGameDataComponent, PlayerScores);
}

void USumoGameDataComponent::SetKillFeedText(const FString& InText)
{
	LastKillFeedText = InText;
	KillFeedCounter++;
	OnKillFeedUpdated.Broadcast(LastKillFeedText);
}

void USumoGameDataComponent::SetCurrentRound(int32 InRound)
{
	CurrentRound = InRound;
	OnRoundChanged.Broadcast(CurrentRound, MaxRounds);
}

void USumoGameDataComponent::SetPlayerScores(const TArray<FSumoPlayerScore>& InScores)
{
	PlayerScores = InScores;
}

void USumoGameDataComponent::OnRep_LastKillFeed()
{
	OnKillFeedUpdated.Broadcast(LastKillFeedText);
}

void USumoGameDataComponent::OnRep_CurrentRound()
{
	OnRoundChanged.Broadcast(CurrentRound, MaxRounds);
}

void USumoGameDataComponent::OnRep_PlayerScores()
{
	// 클라이언트에서 점수 업데이트 시 필요한 처리 (HUD Tick에서 폴링)
}
