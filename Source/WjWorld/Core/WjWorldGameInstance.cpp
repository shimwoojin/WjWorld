// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UWjWorldGameInstance::Init()
{
	Super::Init();

	// SessionManager 생성 및 초기화
	SessionManager = NewObject<USessionManager>(this);
	if (SessionManager)
	{
		SessionManager->Initialize();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: SessionManager created and initialized"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Failed to create SessionManager"));
	}

	// NetworkFailure 핸들러 등록
	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(
			this, &UWjWorldGameInstance::HandleNetworkFailure);
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: NetworkFailure handler registered"));
	}
}

void UWjWorldGameInstance::Shutdown()
{
	// 타이머 정리
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(MigrationTimeoutHandle);
		World->GetTimerManager().ClearTimer(MigrationRetryHandle);
	}

	// NetworkFailure 핸들러 해제
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
	}

	// SessionManager 정리
	if (SessionManager)
	{
		SessionManager->Shutdown();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: SessionManager shutdown"));
	}

	Super::Shutdown();
}

bool UWjWorldGameInstance::CreateRoom(const FRoomSettings& Settings)
{
	if (SessionManager)
	{
		return SessionManager->CreateSession(Settings);
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::FindRooms(ENetworkMode NetworkMode, int32 MaxResults)
{
	if (SessionManager)
	{
		return SessionManager->FindSessions(NetworkMode, MaxResults);
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::JoinRoom(int32 RoomIndex)
{
	if (SessionManager)
	{
		return SessionManager->JoinSession(RoomIndex);
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::StartGame()
{
	if (SessionManager)
	{
		return SessionManager->StartSession();
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::LeaveRoom()
{
	if (SessionManager)
	{
		return SessionManager->DestroySession();
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

bool UWjWorldGameInstance::EndGame()
{
	if (SessionManager)
	{
		return SessionManager->EndSession();
	}

	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: SessionManager is null"));
	return false;
}

// ============================================================================
// 캐싱
// ============================================================================

void UWjWorldGameInstance::CachePlayerList(const TArray<FPlayerDisplayInfo>& PlayerList)
{
	MigrationContext.CachedPlayerList = PlayerList;

	// 로컬 플레이어 ID 캐시
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerState)
	{
		MigrationContext.LocalPlayerID = PC->PlayerState->GetPlayerId();
	}

	UE_LOG(LogWjWorld, Verbose, TEXT("WjWorldGameInstance: Cached %d players (LocalID: %d)"),
		PlayerList.Num(), MigrationContext.LocalPlayerID);
}

void UWjWorldGameInstance::CacheRoomSettings(const FRoomSettings& Settings)
{
	MigrationContext.CachedRoomSettings = Settings;
	UE_LOG(LogWjWorld, Verbose, TEXT("WjWorldGameInstance: Cached room settings '%s'"), *Settings.RoomName);
}

// ============================================================================
// NetworkFailure 감지
// ============================================================================

void UWjWorldGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameInstance: HandleNetworkFailure called - Type: %d, Error: %s"),
		static_cast<int32>(FailureType), *ErrorString);

	// 이미 마이그레이션 중이면 무시
	if (MigrationContext.bIsMigrating)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameInstance: NetworkFailure ignored - already migrating"));
		return;
	}

	// 자신이 호스트(서버)면 마이그레이션 불필요
	if (World && World->GetAuthGameMode() != nullptr)
	{
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: NetworkFailure on server - ignoring (Type: %d, Error: %s)"),
			static_cast<int32>(FailureType), *ErrorString);
		return;
	}

	// 관련 Failure 타입만 처리
	switch (FailureType)
	{
	case ENetworkFailure::ConnectionLost:
	case ENetworkFailure::FailureReceived:
	case ENetworkFailure::ConnectionTimeout:
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameInstance: NetworkFailure detected - Type: %d, Error: %s. Beginning host migration."),
			static_cast<int32>(FailureType), *ErrorString);
		BeginHostMigration();
		break;
	default:
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: NetworkFailure type %d not handled for migration"),
			static_cast<int32>(FailureType));
		break;
	}
}

// ============================================================================
// 호스트 마이그레이션
// ============================================================================

void UWjWorldGameInstance::BeginHostMigration()
{
	MigrationContext.bIsMigrating = true;
	MigrationContext.MigrationStartTime = FPlatformTime::Seconds();
	MigrationContext.RetryCount = 0;
	SetMigrationState(EHostMigrationState::Detected);

	// 마이그레이션 태그 생성 (모든 클라이언트 동일)
	MigrationContext.MigrationSessionTag = GenerateMigrationTag();

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: Host migration started - Tag: '%s'"),
		*MigrationContext.MigrationSessionTag);

	// 기존 세션 파괴
	if (SessionManager)
	{
		SessionManager->DestroySession();
	}

	// 타임아웃 타이머 설정
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(MigrationTimeoutHandle,
			FTimerDelegate::CreateUObject(this, &UWjWorldGameInstance::OnMigrationTimeout),
			FHostMigrationContext::MigrationTimeout, false);
	}

	// 새 호스트 결정
	MigrationContext.bIsNewHost = DetermineIfNewHost();

	if (MigrationContext.bIsNewHost)
	{
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: This client is the new host"));
		MigrationCreateSession();
	}
	else
	{
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: This client is NOT the new host - waiting 3s before searching"));

		// 3초 대기 후 검색 시작
		if (World)
		{
			World->GetTimerManager().SetTimer(MigrationRetryHandle,
				FTimerDelegate::CreateUObject(this, &UWjWorldGameInstance::MigrationSearchForNewHost),
				3.0f, false);
		}
	}
}

bool UWjWorldGameInstance::DetermineIfNewHost() const
{
	const TArray<FPlayerDisplayInfo>& CachedPlayers = MigrationContext.CachedPlayerList;

	if (CachedPlayers.Num() == 0)
	{
		// 캐시가 비어있으면 자신이 호스트
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameInstance: No cached players - assuming new host"));
		return true;
	}

	// PlayerID 오름차순 정렬 (이전 호스트 제외)
	TArray<FPlayerDisplayInfo> Candidates;
	for (const FPlayerDisplayInfo& Info : CachedPlayers)
	{
		if (!Info.bIsHost) // 이전 호스트 제외
		{
			Candidates.Add(Info);
		}
	}

	if (Candidates.Num() == 0)
	{
		// 모든 플레이어가 호스트? 자신이 새 호스트
		return true;
	}

	// PlayerID 오름차순 정렬
	Candidates.Sort([](const FPlayerDisplayInfo& A, const FPlayerDisplayInfo& B)
	{
		return A.PlayerID < B.PlayerID;
	});

	// 첫 번째 = 새 호스트
	return (Candidates[0].PlayerID == MigrationContext.LocalPlayerID);
}

FString UWjWorldGameInstance::GenerateMigrationTag() const
{
	// 결정론적 태그: "MIG_{RoomName}_{OriginalHostPlayerID}"
	FString RoomName = MigrationContext.CachedRoomSettings.RoomName;
	int32 OriginalHostID = -1;

	for (const FPlayerDisplayInfo& Info : MigrationContext.CachedPlayerList)
	{
		if (Info.bIsHost)
		{
			OriginalHostID = Info.PlayerID;
			break;
		}
	}

	return FString::Printf(TEXT("MIG_%s_%d"), *RoomName, OriginalHostID);
}

void UWjWorldGameInstance::MigrationCreateSession()
{
	SetMigrationState(EHostMigrationState::CreatingSession);

	if (!SessionManager)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: MigrationCreateSession - SessionManager is null"));
		MigrationFailed();
		return;
	}

	// 콜백 바인딩
	SessionManager->OnRoomCreatedEvent.AddDynamic(this, &UWjWorldGameInstance::OnMigrationSessionCreated);

	bool bSuccess = SessionManager->CreateMigrationSession(
		MigrationContext.CachedRoomSettings,
		MigrationContext.MigrationSessionTag);

	if (!bSuccess)
	{
		SessionManager->OnRoomCreatedEvent.RemoveDynamic(this, &UWjWorldGameInstance::OnMigrationSessionCreated);
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Failed to create migration session"));
		MigrationRetryOrFail();
	}
}

void UWjWorldGameInstance::OnMigrationSessionCreated(bool bWasSuccessful)
{
	if (SessionManager)
	{
		SessionManager->OnRoomCreatedEvent.RemoveDynamic(this, &UWjWorldGameInstance::OnMigrationSessionCreated);
	}

	if (bWasSuccessful)
	{
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: Migration session created - ServerTraveling to WaitingRoom"));
		SetMigrationState(EHostMigrationState::Complete);

		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(MigrationTimeoutHandle);
			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			World->ServerTravel(Settings->GetWaitingRoomOpenLevelURL());
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Migration session creation failed"));
		MigrationRetryOrFail();
	}
}

void UWjWorldGameInstance::MigrationSearchForNewHost()
{
	SetMigrationState(EHostMigrationState::SearchingSession);

	if (!SessionManager)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: MigrationSearchForNewHost - SessionManager is null"));
		MigrationFailed();
		return;
	}

	// 콜백 바인딩
	SessionManager->OnRoomsFoundEvent.AddDynamic(this, &UWjWorldGameInstance::OnMigrationSearchResult);

	bool bSuccess = SessionManager->FindMigrationSession(MigrationContext.MigrationSessionTag);

	if (!bSuccess)
	{
		SessionManager->OnRoomsFoundEvent.RemoveDynamic(this, &UWjWorldGameInstance::OnMigrationSearchResult);
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Failed to start migration session search"));
		MigrationRetryOrFail();
	}
}

void UWjWorldGameInstance::OnMigrationSearchResult(bool bWasSuccessful, const TArray<FRoomInfo>& Rooms)
{
	if (SessionManager)
	{
		SessionManager->OnRoomsFoundEvent.RemoveDynamic(this, &UWjWorldGameInstance::OnMigrationSearchResult);
	}

	if (bWasSuccessful)
	{
		// JoinSession은 FindMigrationSession 내부에서 자동으로 호출됨
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: Migration session found and joining"));
		SetMigrationState(EHostMigrationState::JoiningSession);

		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(MigrationTimeoutHandle);
		}
		// JoinSession → ClientTravel이 자동으로 발생
	}
	else
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameInstance: Migration session not found - will retry"));
		MigrationRetryOrFail();
	}
}

void UWjWorldGameInstance::MigrationRetryOrFail()
{
	MigrationContext.RetryCount++;

	// 타임아웃 체크
	double ElapsedTime = FPlatformTime::Seconds() - MigrationContext.MigrationStartTime;
	if (ElapsedTime >= FHostMigrationContext::MigrationTimeout)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Migration timed out after %.1f seconds"), ElapsedTime);
		MigrationFailed();
		return;
	}

	// 최대 재시도 횟수 체크
	if (MigrationContext.RetryCount >= FHostMigrationContext::MaxRetryCount)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Migration max retries (%d) reached"), FHostMigrationContext::MaxRetryCount);
		MigrationFailed();
		return;
	}

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: Migration retry %d/%d in %.1f seconds"),
		MigrationContext.RetryCount, FHostMigrationContext::MaxRetryCount, FHostMigrationContext::RetryInterval);

	UWorld* World = GetWorld();
	if (World)
	{
		if (MigrationContext.bIsNewHost)
		{
			World->GetTimerManager().SetTimer(MigrationRetryHandle,
				FTimerDelegate::CreateUObject(this, &UWjWorldGameInstance::MigrationCreateSession),
				FHostMigrationContext::RetryInterval, false);
		}
		else
		{
			World->GetTimerManager().SetTimer(MigrationRetryHandle,
				FTimerDelegate::CreateUObject(this, &UWjWorldGameInstance::MigrationSearchForNewHost),
				FHostMigrationContext::RetryInterval, false);
		}
	}
}

void UWjWorldGameInstance::OnMigrationTimeout()
{
	UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameInstance: Migration timeout (%.0f seconds)"),
		FHostMigrationContext::MigrationTimeout);
	MigrationFailed();
}

void UWjWorldGameInstance::MigrationFailed()
{
	SetMigrationState(EHostMigrationState::Failed);

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(MigrationTimeoutHandle);
		World->GetTimerManager().ClearTimer(MigrationRetryHandle);
	}

	UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameInstance: Migration failed - returning to lobby"));

	// 세션 정리
	if (SessionManager)
	{
		SessionManager->DestroySession();
	}

	// 마이그레이션 컨텍스트 리셋
	MigrationContext.Reset();
	SetMigrationState(EHostMigrationState::None);

	// 로비로 복귀
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	UGameplayStatics::OpenLevel(this, FName(*Settings->GetLobbyMapPath()));
}

void UWjWorldGameInstance::SetMigrationState(EHostMigrationState NewState)
{
	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameInstance: Migration state: %d → %d"),
		static_cast<int32>(MigrationState), static_cast<int32>(NewState));
	MigrationState = NewState;
}
