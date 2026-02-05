// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Session/SessionManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "WjWorldLogCategories.h"

USessionManager::USessionManager()
{
}

void USessionManager::Initialize()
{
	// Online Subsystem 가져오기 - 우선순위: Steam > NULL
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
	if (!OnlineSubsystem)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Steam subsystem not available, trying NULL subsystem"));
		OnlineSubsystem = IOnlineSubsystem::Get(NULL_SUBSYSTEM);
	}

	if (OnlineSubsystem)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnlineSubsystem found - Name: %s"), *OnlineSubsystem->GetSubsystemName().ToString());

		SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			// 델리게이트 바인딩
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USessionManager::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USessionManager::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USessionManager::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &USessionManager::OnDestroySessionComplete);
			SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &USessionManager::OnStartSessionComplete);
			SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &USessionManager::OnEndSessionComplete);

			UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Initialized successfully with %s"), *OnlineSubsystem->GetSubsystemName().ToString());
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to get SessionInterface from %s"), *OnlineSubsystem->GetSubsystemName().ToString());
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: No OnlineSubsystem available (Steam and NULL both failed)"));
	}
}

void USessionManager::Shutdown()
{
	if (SessionInterface.IsValid())
	{
		// 델리게이트 언바인딩
		SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnDestroySessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnStartSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnEndSessionCompleteDelegates.RemoveAll(this);

		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Shutdown"));
	}
}

void USessionManager::ApplyNetDriverForMode(ENetworkMode Mode)
{
	if (!GEngine) return;

	for (FNetDriverDefinition& Def : GEngine->NetDriverDefinitions)
	{
		if (Def.DefName == FName(TEXT("GameNetDriver")))
		{
			if (Mode == ENetworkMode::LAN)
			{
				// SocketSubsystemSteamIP가 기본 소켓을 Steam으로 오버라이드하므로
				// IpNetDriver 대신 WjWorldLanNetDriver 사용 (PLATFORM_SOCKETSUBSYSTEM 명시)
				Def.DriverClassName = FName(TEXT("/Script/WjWorld.WjWorldLanNetDriver"));
				Def.DriverClassNameFallback = FName(TEXT("/Script/OnlineSubsystemUtils.IpNetDriver"));
			}
			else // Steam
			{
				Def.DriverClassName = FName(TEXT("/Script/SocketSubsystemSteamIP.SteamNetDriver"));
				Def.DriverClassNameFallback = FName(TEXT("/Script/OnlineSubsystemUtils.IpNetDriver"));
			}
			UE_LOG(LogWjWorld, Log, TEXT("SessionManager: NetDriver set to '%s' for %s mode"),
				*Def.DriverClassName.ToString(),
				Mode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));
			break;
		}
	}
}

bool USessionManager::CreateSession(const FRoomSettings& Settings)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	// ⭐ 중요: 기존 세션이 있다면 먼저 완전히 제거
	auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Existing session found. Destroying synchronously..."));
		
		SessionInterface->DestroySession(SESSION_NAME);
		
		int32 WaitCount = 0;
		while (SessionInterface->GetNamedSession(SESSION_NAME) != nullptr && WaitCount < 100)
		{
			FPlatformProcess::Sleep(0.01f);
			WaitCount++;
		}
		
		if (SessionInterface->GetNamedSession(SESSION_NAME) != nullptr)
		{
			UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to destroy existing session"));
			return false;
		}
		
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Existing session destroyed successfully"));
	}

	// 네트워크 모드에 맞게 NetDriver 전환 (Listen Server가 올바른 드라이버 사용)
	ApplyNetDriverForMode(Settings.NetworkMode);

	// 세션 설정
	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = Settings.MaxPlayers;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = Settings.bAllowJoinInProgress;
	SessionSettings.bAllowJoinViaPresence = false;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;

	// 네트워크 모드에 따른 설정
	// Steam: bUsesPresence와 bUseLobbiesIfAvailable은 반드시 같은 값이어야 함
	if (Settings.NetworkMode == ENetworkMode::Steam)
	{
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
	}
	else // LAN
	{
		SessionSettings.bIsLANMatch = true;
		SessionSettings.bUsesPresence = false;
		SessionSettings.bUseLobbiesIfAvailable = false;
	}

	// 커스텀 데이터 설정
	SessionSettings.Set(FName("ROOM_NAME"), Settings.RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("GAME_MODE"), Settings.GameMode, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("MAP_NAME"), Settings.MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("IS_PRIVATE"), Settings.bIsPrivate, EOnlineDataAdvertisementType::ViaOnlineService);

	if (Settings.bIsPrivate && !Settings.Password.IsEmpty())
	{
		SessionSettings.Set(FName("PASSWORD"), Settings.Password, EOnlineDataAdvertisementType::ViaOnlineService);
	}

	// 세션 생성 시도
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), SESSION_NAME, SessionSettings);

	if (bSuccess)
	{
		LastRoomSettings = Settings;
		bIsHost = true;
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Creating %s session '%s'"),
			Settings.NetworkMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"),
			*Settings.RoomName);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to create session"));
	}

	return bSuccess;
}

bool USessionManager::FindSessions(ENetworkMode NetworkMode, int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	// 이전 검색이 진행 중이면 대기열에 등록 (완료 후 자동 실행)
	if (bIsSearchInProgress)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Previous search in progress, queuing new %s search"),
			NetworkMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));
		PendingSearchRequest = TPair<ENetworkMode, int32>(NetworkMode, MaxSearchResults);
		return true;
	}

	// 검색 네트워크 모드 저장 (JoinSession에서 NetDriver 전환용)
	LastSearchNetworkMode = NetworkMode;

	// 검색 설정
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;

	// 네트워크 모드에 따른 검색 설정
	if (NetworkMode == ENetworkMode::Steam)
	{
		SessionSearch->bIsLanQuery = false;
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}
	else // LAN
	{
		SessionSearch->bIsLanQuery = true;
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
	}

	// 검색 시작
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());

	if (bSuccess)
	{
		bIsSearchInProgress = true;
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Searching for %s sessions..."),
			NetworkMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to start session search"));
	}

	return bSuccess;
}

bool USessionManager::JoinSession(int32 RoomIndex)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	if (!SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: No search results available"));
		return false;
	}

	if (!SessionSearch->SearchResults.IsValidIndex(RoomIndex))
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Invalid room index %d"), RoomIndex);
		return false;
	}

	// ⭐ 중요: 기존 세션이 있다면 먼저 제거
	auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Destroying existing session before joining..."));
		
		SessionInterface->DestroySession(SESSION_NAME);
		
		int32 WaitCount = 0;
		while (SessionInterface->GetNamedSession(SESSION_NAME) != nullptr && WaitCount < 100)
		{
			FPlatformProcess::Sleep(0.01f);
			WaitCount++;
		}
		
		if (SessionInterface->GetNamedSession(SESSION_NAME) != nullptr)
		{
			UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to destroy existing session"));
			return false;
		}
		
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Existing session destroyed successfully"));
	}

	LastJoinedRoomIndex = RoomIndex;

	// 선택된 세션에 참가
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), SESSION_NAME, SessionSearch->SearchResults[RoomIndex]);

	if (bSuccess)
	{
		bIsHost = false;
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Joining session at index %d"), RoomIndex);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to join session"));
	}

	return bSuccess;
}

bool USessionManager::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	if (!bIsHost)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Only host can start session"));
		return false;
	}

	bool bSuccess = SessionInterface->StartSession(SESSION_NAME);

	if (bSuccess)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Starting session"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to start session"));
	}

	return bSuccess;
}

bool USessionManager::EndSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	// 세션이 존재하고 InProgress 상태인지 확인
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (!ExistingSession)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: EndSession - No session exists"));
		return false;
	}

	if (ExistingSession->SessionState != EOnlineSessionState::InProgress)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: EndSession - Session not InProgress (state: %d), skipping"),
			static_cast<int32>(ExistingSession->SessionState));
		return true; // 이미 종료된 상태이므로 성공으로 간주
	}

	bool bSuccess = SessionInterface->EndSession(SESSION_NAME);

	if (bSuccess)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Ending session (InProgress → Ended)"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to end session"));
	}

	return bSuccess;
}

bool USessionManager::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	bool bSuccess = SessionInterface->DestroySession(SESSION_NAME);

	if (bSuccess)
	{
		bIsInSession = false;
		bIsHost = false;
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Destroying session"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to destroy session"));
	}

	return bSuccess;
}

void USessionManager::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnCreateSessionComplete - Success: %d, SessionName: %s"), bWasSuccessful, *SessionName.ToString());

	if (bWasSuccessful)
	{
		bIsInSession = true;
		bIsHost = true;
	}

	OnRoomCreatedEvent.Broadcast(bWasSuccessful);
}

void USessionManager::OnFindSessionsComplete(bool bWasSuccessful)
{
	bIsSearchInProgress = false;

	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnFindSessionsComplete - Success: %d"), bWasSuccessful);

	// 대기 중인 검색 요청이 있으면 즉시 실행
	if (PendingSearchRequest.IsSet())
	{
		ENetworkMode PendingMode = PendingSearchRequest.GetValue().Key;
		int32 PendingMax = PendingSearchRequest.GetValue().Value;
		PendingSearchRequest.Reset();
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Executing pending %s search"),
			PendingMode == ENetworkMode::Steam ? TEXT("Steam") : TEXT("LAN"));
		FindSessions(PendingMode, PendingMax);
		return;
	}

	// 마이그레이션 세션 검색인 경우
	if (!PendingMigrationTag.IsEmpty())
	{
		FString SearchTag = PendingMigrationTag;
		PendingMigrationTag.Empty();

		if (bWasSuccessful && SessionSearch.IsValid())
		{
			for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
			{
				FString FoundTag;
				SessionSearch->SearchResults[i].Session.SessionSettings.Get(FName("MIGRATION_TAG"), FoundTag);

				if (FoundTag == SearchTag)
				{
					UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Found migration session with tag '%s' at index %d"), *SearchTag, i);
					JoinSession(i);
					return;
				}
			}

			UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Migration session with tag '%s' not found among %d results"),
				*SearchTag, SessionSearch->SearchResults.Num());
		}

		// 마이그레이션 세션 미발견 → 빈 결과로 브로드캐스트하여 GameInstance가 재시도 처리
		OnRoomsFoundEvent.Broadcast(false, TArray<FRoomInfo>());
		return;
	}

	// 일반 세션 검색
	TArray<FRoomInfo> Rooms;

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Found %d sessions"), SessionSearch->SearchResults.Num());

		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];

			UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Session[%d] Details:"), i);
			UE_LOG(LogWjWorld, Log, TEXT("  - IsValid: %d"), Result.IsValid());
			UE_LOG(LogWjWorld, Log, TEXT("  - Ping: %d"), Result.PingInMs);
			UE_LOG(LogWjWorld, Log, TEXT("  - NumPublicConnections: %d"), Result.Session.SessionSettings.NumPublicConnections);
			UE_LOG(LogWjWorld, Log, TEXT("  - NumOpenPublicConnections: %d"), Result.Session.NumOpenPublicConnections);

			FRoomInfo RoomInfo = ConvertSearchResultToRoomInfo(Result, i);
			UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Room[%d] Name='%s' Mode='%s' Players=%d/%d"),
				i, *RoomInfo.RoomName, *RoomInfo.GameMode,
				RoomInfo.CurrentPlayers, RoomInfo.MaxPlayers);

			Rooms.Add(RoomInfo);
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: No sessions found or search failed"));
	}

	OnRoomsFoundEvent.Broadcast(bWasSuccessful, Rooms);
}

void USessionManager::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);
	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnJoinSessionComplete - Success: %d, Result: %d"), bWasSuccessful, (int32)Result);

	if (bWasSuccessful)
	{
		bIsInSession = true;

		if (SessionInterface.IsValid())
		{
			FString ConnectInfo;
			SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo);

			UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: GetResolvedConnectString returned: '%s'"), *ConnectInfo);

			// ⭐ 포트가 0이면 7777로 교체
			if (ConnectInfo.Contains(TEXT(":0")))
			{
				// IP 주소 추출 (":"앞부분)
				FString IPAddress;
				FString PortString;

				if (ConnectInfo.Split(TEXT(":"), &IPAddress, &PortString))
				{
					// 올바른 포트로 재조립
					ConnectInfo = IPAddress + TEXT(":7777");
					UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Fixed ConnectInfo: %s"), *ConnectInfo);
				}
				else
				{
					// Split 실패 시 Fallback
					ConnectInfo = TEXT("127.0.0.1:7777");
					UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Using fallback: %s"), *ConnectInfo);
				}
			}

			// 검색 시 사용한 네트워크 모드에 맞게 NetDriver 전환
			ApplyNetDriverForMode(LastSearchNetworkMode);

			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC && !ConnectInfo.IsEmpty())
			{
				UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Traveling to: %s"), *ConnectInfo);
				PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				UE_LOG(LogWjWorld, Error, TEXT("SessionManager: ConnectInfo is empty or PC is null"));
			}
		}
	}

	OnRoomJoinedEvent.Broadcast(bWasSuccessful);
}

void USessionManager::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnDestroySessionComplete - Success: %d"), bWasSuccessful);

	if (bWasSuccessful)
	{
		bIsInSession = false;
		bIsHost = false;
	}

	OnRoomDestroyedEvent.Broadcast(bWasSuccessful);
}

void USessionManager::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnStartSessionComplete - Success: %d"), bWasSuccessful);

	OnRoomStartedEvent.Broadcast(bWasSuccessful);
}

void USessionManager::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnEndSessionComplete - Success: %d"), bWasSuccessful);

	OnRoomEndedEvent.Broadcast(bWasSuccessful);
}

bool USessionManager::CreateMigrationSession(const FRoomSettings& Settings, const FString& MigrationTag)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	// 기존 세션 제거
	auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("SessionManager: Migration - Destroying existing session..."));

		SessionInterface->DestroySession(SESSION_NAME);

		int32 WaitCount = 0;
		while (SessionInterface->GetNamedSession(SESSION_NAME) != nullptr && WaitCount < 100)
		{
			FPlatformProcess::Sleep(0.01f);
			WaitCount++;
		}

		if (SessionInterface->GetNamedSession(SESSION_NAME) != nullptr)
		{
			UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Migration - Failed to destroy existing session"));
			return false;
		}
	}

	// 네트워크 모드에 맞게 NetDriver 전환
	ApplyNetDriverForMode(Settings.NetworkMode);

	// 세션 설정 (CreateSession과 동일하되 MIGRATION_TAG 추가)
	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = Settings.MaxPlayers;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = false;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;

	// 네트워크 모드에 따른 설정
	// Steam: bUsesPresence와 bUseLobbiesIfAvailable은 반드시 같은 값이어야 함
	if (Settings.NetworkMode == ENetworkMode::Steam)
	{
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
	}
	else // LAN
	{
		SessionSettings.bIsLANMatch = true;
		SessionSettings.bUsesPresence = false;
		SessionSettings.bUseLobbiesIfAvailable = false;
	}

	// 커스텀 데이터 설정
	SessionSettings.Set(FName("ROOM_NAME"), Settings.RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("GAME_MODE"), Settings.GameMode, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("MAP_NAME"), Settings.MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(FName("IS_PRIVATE"), Settings.bIsPrivate, EOnlineDataAdvertisementType::ViaOnlineService);

	// 마이그레이션 태그 추가
	SessionSettings.Set(FName("MIGRATION_TAG"), MigrationTag, EOnlineDataAdvertisementType::ViaOnlineService);

	// 세션 생성
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), SESSION_NAME, SessionSettings);

	if (bSuccess)
	{
		LastRoomSettings = Settings;
		bIsHost = true;
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Creating migration session '%s' with tag '%s'"),
			*Settings.RoomName, *MigrationTag);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to create migration session"));
	}

	return bSuccess;
}

bool USessionManager::FindMigrationSession(const FString& MigrationTag)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	PendingMigrationTag = MigrationTag;

	// 검색 설정
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 50;

	// 네트워크 모드에 따른 검색 설정 (LastRoomSettings에서 가져옴)
	if (LastRoomSettings.NetworkMode == ENetworkMode::Steam)
	{
		SessionSearch->bIsLanQuery = false;
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}
	else // LAN
	{
		SessionSearch->bIsLanQuery = true;
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
	}

	// 검색 시작
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());

	if (bSuccess)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Searching for migration session with tag '%s'"), *MigrationTag);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to start migration session search"));
	}

	return bSuccess;
}

FRoomInfo USessionManager::ConvertSearchResultToRoomInfo(const FOnlineSessionSearchResult& SearchResult, int32 Index)
{
	FRoomInfo RoomInfo;

	// 기본 정보
	RoomInfo.SearchResultIndex = Index;
	
	// ⭐ 수정: 플레이어 수 계산
	// NumOpenPublicConnections = 열린 슬롯 수
	// 현재 플레이어 = 전체 - 열린 슬롯
	int32 MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
	int32 OpenSlots = SearchResult.Session.NumOpenPublicConnections;
	
	RoomInfo.MaxPlayers = MaxPlayers;
	RoomInfo.CurrentPlayers = MaxPlayers - OpenSlots;
	
	// ⭐ 최소 1명 보장 (호스트)
	if (RoomInfo.CurrentPlayers <= 0)
	{
		RoomInfo.CurrentPlayers = 1;
	}
	
	RoomInfo.Ping = SearchResult.PingInMs;

	// 커스텀 데이터
	SearchResult.Session.SessionSettings.Get(FName("ROOM_NAME"), RoomInfo.RoomName);
	SearchResult.Session.SessionSettings.Get(FName("GAME_MODE"), RoomInfo.GameMode);
	SearchResult.Session.SessionSettings.Get(FName("IS_PRIVATE"), RoomInfo.bIsPrivate);

	// 호스트 이름
	if (SearchResult.Session.OwningUserName.IsEmpty())
	{
		RoomInfo.HostName = TEXT("Unknown");
	}
	else
	{
		RoomInfo.HostName = SearchResult.Session.OwningUserName;
	}

	// 진행 상태
	RoomInfo.bInProgress = SearchResult.Session.SessionSettings.bAllowJoinInProgress && (RoomInfo.CurrentPlayers > 0);

	return RoomInfo;
}
