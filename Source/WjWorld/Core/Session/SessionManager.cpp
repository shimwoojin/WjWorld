// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Session/SessionManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "WjWorldLogCategories.h"

USessionManager::USessionManager()
{
}

void USessionManager::Initialize()
{
	// Online Subsystem 가져오기
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			// 델리게이트 바인딩
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USessionManager::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USessionManager::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USessionManager::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &USessionManager::OnDestroySessionComplete);
			SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &USessionManager::OnStartSessionComplete);

			UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Initialized successfully"));
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to get SessionInterface"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: OnlineSubsystem is null"));
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

		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Shutdown"));
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

	// 세션 설정
	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = Settings.MaxPlayers;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = false;
	SessionSettings.bUseLobbiesIfAvailable = false;
	SessionSettings.bAllowJoinInProgress = Settings.bAllowJoinInProgress;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bAllowJoinViaPresence = false;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;

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
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Creating LAN session '%s'"), *Settings.RoomName);
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to create LAN session"));
	}

	return bSuccess;
}

bool USessionManager::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: SessionInterface is invalid"));
		return false;
	}

	// 검색 설정
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = true;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);

	// 검색 시작
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());

	if (bSuccess)
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Searching for LAN sessions..."));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("SessionManager: Failed to start LAN session search"));
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
	UE_LOG(LogWjWorld, Log, TEXT("SessionManager: OnFindSessionsComplete - Success: %d"), bWasSuccessful);

	TArray<FRoomInfo> Rooms;

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogWjWorld, Log, TEXT("SessionManager: Found %d sessions"), SessionSearch->SearchResults.Num());

		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];
			
			// ⭐ 디버깅: 상세 정보 로그
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
