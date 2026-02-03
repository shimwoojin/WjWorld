// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/WaitingRoom/WjWorldGameStateWaitingRoom.h"
#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Core/WjWorldGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "WjWorldLogCategories.h"

AWjWorldGameStateWaitingRoom::AWjWorldGameStateWaitingRoom()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AWjWorldGameStateWaitingRoom::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 방 설정 복제
	DOREPLIFETIME(AWjWorldGameStateWaitingRoom, RoomSettings);
}

void AWjWorldGameStateWaitingRoom::InitializeRoomSettings(const FRoomSettings& Settings)
{
	// Server Only
	if (!HasAuthority())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("GameStateWaitingRoom: InitializeRoomSettings called on non-authority"));
		return;
	}

	RoomSettings = Settings;
	OnRep_RoomSettings(); // 로컬에서도 이벤트 발생

	// GameInstance에 방 설정 캐시 (호스트 마이그레이션용)
	if (UWjWorldGameInstance* GI = Cast<UWjWorldGameInstance>(GetGameInstance()))
	{
		GI->CacheRoomSettings(Settings);
	}

	UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: Room settings initialized - %s"), *Settings.RoomName);
}

TArray<FPlayerDisplayInfo> AWjWorldGameStateWaitingRoom::GetPlayerList() const
{
	TArray<FPlayerDisplayInfo> PlayerList;

	for (APlayerState* PS : PlayerArray)
	{
		AWjWorldPlayerStateBase* WjPS = Cast<AWjWorldPlayerStateBase>(PS);
		if (!WjPS)
		{
			continue;
		}

		// ⭐ 이름이 비어있으면 스킵 (아직 완전히 초기화되지 않음)
		FString PlayerName = WjPS->GetPlayerName();
		if (PlayerName.IsEmpty())
		{
			UE_LOG(LogWjWorld, Verbose, TEXT("GameStateWaitingRoom: Skipping PlayerState with empty name (ID: %d)"), 
				WjPS->GetPlayerId());
			continue;
		}

		FPlayerDisplayInfo Info;
		Info.PlayerName = PlayerName;
		Info.bIsReady = WjPS->IsReady();
		Info.PlayerID = WjPS->GetPlayerId();
		Info.bIsHost = (HostPlayerID != -1 && WjPS->GetPlayerId() == HostPlayerID);

		PlayerList.Add(Info);
	}

	return PlayerList;
}

int32 AWjWorldGameStateWaitingRoom::GetPlayerCount() const
{
	// ⭐ 이름이 있는 PlayerState만 카운트
	int32 Count = 0;
	for (APlayerState* PS : PlayerArray)
	{
		AWjWorldPlayerStateBase* WjPS = Cast<AWjWorldPlayerStateBase>(PS);
		if (WjPS && !WjPS->GetPlayerName().IsEmpty())
		{
			Count++;
		}
	}
	return Count;
}

int32 AWjWorldGameStateWaitingRoom::GetReadyPlayerCount() const
{
	int32 ReadyCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		AWjWorldPlayerStateBase* WjPS = Cast<AWjWorldPlayerStateBase>(PS);
		if (WjPS && !WjPS->GetPlayerName().IsEmpty() && WjPS->IsReady())
		{
			ReadyCount++;
		}
	}

	return ReadyCount;
}

bool AWjWorldGameStateWaitingRoom::AreAllPlayersReady() const
{
	// 유효한 플레이어 수 확인
	int32 ValidPlayerCount = GetPlayerCount();
	if (ValidPlayerCount == 0)
	{
		return false;
	}

	for (APlayerState* PS : PlayerArray)
	{
		AWjWorldPlayerStateBase* WjPS = Cast<AWjWorldPlayerStateBase>(PS);
		if (WjPS && !WjPS->GetPlayerName().IsEmpty() && !WjPS->IsReady())
		{
			return false;
		}
	}

	return true;
}

void AWjWorldGameStateWaitingRoom::OnRep_RoomSettings()
{
	UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: Room settings replicated - %s"), *RoomSettings.RoomName);

	// 클라이언트 측에서도 캐시 (호스트 마이그레이션용)
	if (UWjWorldGameInstance* GI = Cast<UWjWorldGameInstance>(GetGameInstance()))
	{
		GI->CacheRoomSettings(RoomSettings);
	}

	// 이벤트 브로드캐스트
	OnRoomInfoChanged.Broadcast(RoomSettings);
}

void AWjWorldGameStateWaitingRoom::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	AWjWorldPlayerStateBase* WjPS = Cast<AWjWorldPlayerStateBase>(PlayerState);
	if (WjPS)
	{
		FString PlayerName = WjPS->GetPlayerName();

		// ⭐ PlayerName 업데이트 이벤트 구독
		WjPS->OnPlayerNameUpdated.AddDynamic(this, &AWjWorldGameStateWaitingRoom::OnPlayerNameUpdated);

		// ⭐ Ready 상태 변경 이벤트 구독
		WjPS->OnReadyStateChanged.AddDynamic(this, &AWjWorldGameStateWaitingRoom::OnPlayerReadyStateChanged);

		// ⭐ 이름이 비어있으면 아직 초기화 중 (나중에 OnPlayerNameUpdated 호출됨)
		if (PlayerName.IsEmpty())
		{
			UE_LOG(LogWjWorld, Warning, TEXT("GameStateWaitingRoom: Player added with empty name (ID: %d) - Waiting for replication"), 
				WjPS->GetPlayerId());
			return;
		}

		// 첫 번째 플레이어를 호스트로 설정
		if (HostPlayerID == -1)
		{
			HostPlayerID = WjPS->GetPlayerId();
			UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: Host set to PlayerID %d"), HostPlayerID);
		}

		UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: Player added - %s (ID: %d, Total: %d)"),
			*PlayerName, WjPS->GetPlayerId(), GetPlayerCount());
	}

	// 플레이어 목록 변경 브로드캐스트
	BroadcastPlayerListChanged();
}

void AWjWorldGameStateWaitingRoom::RemovePlayerState(APlayerState* PlayerState)
{
	AWjWorldPlayerStateBase* WjPS = Cast<AWjWorldPlayerStateBase>(PlayerState);
	if (WjPS)
	{
		// 이벤트 구독 해제
		WjPS->OnPlayerNameUpdated.RemoveDynamic(this, &AWjWorldGameStateWaitingRoom::OnPlayerNameUpdated);
		WjPS->OnReadyStateChanged.RemoveDynamic(this, &AWjWorldGameStateWaitingRoom::OnPlayerReadyStateChanged);

		UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: Player removed - %s (ID: %d, Remaining: %d)"),
			*WjPS->GetPlayerName(), WjPS->GetPlayerId(), GetPlayerCount() - 1);

		// 호스트가 나갔다면 새로운 호스트 지정
		if (WjPS->GetPlayerId() == HostPlayerID && PlayerArray.Num() > 1)
		{
			// 나가는 플레이어를 제외한 첫 번째 유효한 플레이어를 새 호스트로
			for (APlayerState* PS : PlayerArray)
			{
				AWjWorldPlayerStateBase* NextHost = Cast<AWjWorldPlayerStateBase>(PS);
				if (NextHost && NextHost != WjPS && !NextHost->GetPlayerName().IsEmpty())
				{
					HostPlayerID = NextHost->GetPlayerId();
					UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: New host set to PlayerID %d"), HostPlayerID);
					break;
				}
			}
		}
	}

	Super::RemovePlayerState(PlayerState);

	// 플레이어 목록 변경 브로드캐스트
	BroadcastPlayerListChanged();
}

void AWjWorldGameStateWaitingRoom::OnPlayerNameUpdated(const FString& PlayerName)
{
	UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: OnPlayerNameUpdated called - %s"), *PlayerName);

	// 플레이어 목록 재브로드캐스트
	BroadcastPlayerListChanged();
}

void AWjWorldGameStateWaitingRoom::OnPlayerReadyStateChanged(int32 PlayerID, bool bIsReady)
{
	UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: OnPlayerReadyStateChanged - PlayerID: %d, IsReady: %s"),
		PlayerID, bIsReady ? TEXT("true") : TEXT("false"));

	// 플레이어 목록 재브로드캐스트 (Ready 상태 포함)
	BroadcastPlayerListChanged();
}

void AWjWorldGameStateWaitingRoom::BroadcastPlayerListChanged()
{
	TArray<FPlayerDisplayInfo> PlayerList = GetPlayerList();

	UE_LOG(LogWjWorld, Log, TEXT("GameStateWaitingRoom: Broadcasting player list change - %d players"), PlayerList.Num());

	// GameInstance에 플레이어 목록 캐시 (호스트 마이그레이션용)
	if (UWjWorldGameInstance* GI = Cast<UWjWorldGameInstance>(GetGameInstance()))
	{
		GI->CachePlayerList(PlayerList);
	}

	// 이벤트 브로드캐스트
	OnPlayerListChanged.Broadcast(PlayerList);
}
