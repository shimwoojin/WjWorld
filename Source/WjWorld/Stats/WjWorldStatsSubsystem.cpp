// Fill out your copyright notice in the Description page of Project Settings.

#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"
#include "WjWorldLogCategories.h"

#if WITH_STEAM
#include "steam/steam_api.h"
#endif

const FString UWjWorldStatsSubsystem::StatsConfigSection = TEXT("WjWorldStats");

void UWjWorldStatsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: Initialize"));

	RequestCurrentStats();
}

void UWjWorldStatsSubsystem::Deinitialize()
{
	// 종료 시 저장
	StoreStats();

	Super::Deinitialize();
}

void UWjWorldStatsSubsystem::RequestCurrentStats()
{
#if WITH_STEAM
	ISteamUserStats* SteamStats = ::SteamUserStats();
	if (SteamStats)
	{
		// Steamworks v161: RequestCurrentStats()가 제거됨
		// Steam 클라이언트가 자동으로 스탯을 동기화하므로 즉시 사용 가능
		bLocalStatsLoaded = true;

		// 로컬 캐시 업데이트
		const TArray<FMinigameStatDescriptor>& Descriptors = GetAllMinigameDescriptors();
		for (const FMinigameStatDescriptor& Desc : Descriptors)
		{
			for (const FMinigameStatEntry& Entry : Desc.Stats)
			{
				int32 Value = 0;
				SteamStats->GetStat(TCHAR_TO_UTF8(*Entry.StatName.ToString()), &Value);
				LocalStats.Add(Entry.StatName, Value);
			}
		}

		OnLocalStatsReady.Broadcast();
		UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: Steam stats loaded (auto-synced by client)"));
	}
	else
	{
		// Steam 초기화 안 됨 - 폴백
		LoadLocalStatsFromConfig();
		OnLocalStatsReady.Broadcast();
	}
#else
	LoadLocalStatsFromConfig();
	OnLocalStatsReady.Broadcast();
	UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: Loaded local stats from config (non-Steam)"));
#endif
}

int32 UWjWorldStatsSubsystem::GetLocalStat(FName StatName) const
{
#if WITH_STEAM
	ISteamUserStats* SteamStats = ::SteamUserStats();
	if (SteamStats && bLocalStatsLoaded)
	{
		int32 Value = 0;
		if (SteamStats->GetStat(TCHAR_TO_UTF8(*StatName.ToString()), &Value))
		{
			return Value;
		}
	}
	// 폴백
#endif

	const int32* Found = LocalStats.Find(StatName);
	return Found ? *Found : 0;
}

void UWjWorldStatsSubsystem::IncrementLocalStat(FName StatName, int32 Delta)
{
	int32 CurrentValue = GetLocalStat(StatName);
	int32 NewValue = CurrentValue + Delta;

#if WITH_STEAM
	ISteamUserStats* SteamStats = ::SteamUserStats();
	if (SteamStats && bLocalStatsLoaded)
	{
		SteamStats->SetStat(TCHAR_TO_UTF8(*StatName.ToString()), NewValue);
	}
#endif

	LocalStats.FindOrAdd(StatName) = NewValue;

	UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: %s incremented %d -> %d"), *StatName.ToString(), CurrentValue, NewValue);
}

void UWjWorldStatsSubsystem::StoreStats()
{
#if WITH_STEAM
	ISteamUserStats* SteamStats = ::SteamUserStats();
	if (SteamStats && bLocalStatsLoaded)
	{
		SteamStats->StoreStats();
		UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: StoreStats called (Steam)"));
		return;
	}
#endif

	SaveLocalStatsToConfig();
	UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: StoreStats called (config fallback)"));
}

void UWjWorldStatsSubsystem::RequestUserStats(const FUniqueNetIdRepl& UserId)
{
	if (!UserId.IsValid())
	{
		return;
	}

	FString UserIdStr = UserId.ToString();

#if WITH_STEAM
	ISteamUserStats* SteamStats = ::SteamUserStats();
	if (SteamStats)
	{
		// FUniqueNetIdRepl에서 CSteamID 추출
		uint64 SteamId64 = FCString::Atoi64(*UserIdStr);
		CSteamID SteamId(SteamId64);
		SteamStats->RequestUserStats(SteamId);
		UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: Requested user stats for %s"), *UserIdStr);
		return;
	}
#endif

	// 폴백: 타 유저 스탯은 불가능하므로 빈 데이터로 즉시 완료
	UserStatsCache.FindOrAdd(UserIdStr);
	ReadyUserIds.Add(UserIdStr);

	FUniqueNetIdRepl UserIdCopy = UserId;
	OnUserStatsReceived.Broadcast(UserIdCopy);
}

int32 UWjWorldStatsSubsystem::GetUserStat(const FUniqueNetIdRepl& UserId, FName StatName) const
{
	if (!UserId.IsValid())
	{
		return 0;
	}

	FString UserIdStr = UserId.ToString();

#if WITH_STEAM
	ISteamUserStats* SteamStats = ::SteamUserStats();
	if (SteamStats && ReadyUserIds.Contains(UserIdStr))
	{
		uint64 SteamId64 = FCString::Atoi64(*UserIdStr);
		CSteamID SteamId(SteamId64);
		int32 Value = 0;
		if (SteamStats->GetUserStat(SteamId, TCHAR_TO_UTF8(*StatName.ToString()), &Value))
		{
			return Value;
		}
	}
#endif

	const TMap<FName, int32>* UserStats = UserStatsCache.Find(UserIdStr);
	if (UserStats)
	{
		const int32* Found = UserStats->Find(StatName);
		return Found ? *Found : 0;
	}
	return 0;
}

bool UWjWorldStatsSubsystem::IsUserStatsReady(const FUniqueNetIdRepl& UserId) const
{
	if (!UserId.IsValid())
	{
		return false;
	}
	return ReadyUserIds.Contains(UserId.ToString());
}

void UWjWorldStatsSubsystem::SaveLocalStatsToConfig()
{
	for (const auto& Pair : LocalStats)
	{
		GConfig->SetInt(*StatsConfigSection, *Pair.Key.ToString(), Pair.Value, GGameIni);
	}
	GConfig->Flush(false, GGameIni);
}

void UWjWorldStatsSubsystem::LoadLocalStatsFromConfig()
{
	LocalStats.Empty();

	// 등록된 모든 스탯 이름을 순회하여 로드
	const TArray<FMinigameStatDescriptor>& Descriptors = GetAllMinigameDescriptors();
	for (const FMinigameStatDescriptor& Desc : Descriptors)
	{
		for (const FMinigameStatEntry& Entry : Desc.Stats)
		{
			int32 Value = 0;
			if (GConfig->GetInt(*StatsConfigSection, *Entry.StatName.ToString(), Value, GGameIni))
			{
				LocalStats.Add(Entry.StatName, Value);
			}
			else
			{
				LocalStats.Add(Entry.StatName, 0);
			}
		}
	}
}

#if WITH_STEAM
void UWjWorldStatsSubsystem::OnSteamUserStatsReceived(uint64 SteamId, bool bSuccess)
{
	ISteamUser* SteamUserAPI = ::SteamUser();
	if (!SteamUserAPI)
	{
		return;
	}

	CSteamID LocalSteamId = SteamUserAPI->GetSteamID();

	if (SteamId == LocalSteamId.ConvertToUint64())
	{
		// 로컬 유저 스탯 수신
		bLocalStatsLoaded = bSuccess;
		if (bSuccess)
		{
			// 로컬 캐시도 업데이트
			ISteamUserStats* SteamStats = ::SteamUserStats();
			if (SteamStats)
			{
				const TArray<FMinigameStatDescriptor>& Descriptors = GetAllMinigameDescriptors();
				for (const FMinigameStatDescriptor& Desc : Descriptors)
				{
					for (const FMinigameStatEntry& Entry : Desc.Stats)
					{
						int32 Value = 0;
						SteamStats->GetStat(TCHAR_TO_UTF8(*Entry.StatName.ToString()), &Value);
						LocalStats.Add(Entry.StatName, Value);
					}
				}
			}
		}
		OnLocalStatsReady.Broadcast();
		UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: Local stats received (success=%d)"), bSuccess);
	}
	else
	{
		// 타 유저 스탯 수신
		FString UserIdStr = FString::Printf(TEXT("%llu"), SteamId);
		ReadyUserIds.Add(UserIdStr);

		FUniqueNetIdRepl UserId;
		// 타 유저의 OnUserStatsReceived 브로드캐스트
		OnUserStatsReceived.Broadcast(UserId);
		UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: User stats received for %s (success=%d)"), *UserIdStr, bSuccess);
	}
}

void UWjWorldStatsSubsystem::OnSteamUserStatsStored(uint64 SteamId, bool bSuccess)
{
	UE_LOG(LogWjWorld, Log, TEXT("StatsSubsystem: Stats stored (success=%d)"), bSuccess);
}
#endif
