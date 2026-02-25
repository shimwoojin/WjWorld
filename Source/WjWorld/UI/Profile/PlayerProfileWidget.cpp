// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Profile/PlayerProfileWidget.h"
#include "UI/Profile/CharacterPreviewActor.h"
#include "Stats/WjWorldStatsSubsystem.h"
#include "Stats/WjWorldStatTypes.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UPlayerProfileWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UPlayerProfileWidget::OnCloseClicked);
	}
}

void UPlayerProfileWidget::NativeDestruct()
{
	// 스탯 서브시스템 델리게이트 정리
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWjWorldStatsSubsystem* StatsSub = GI->GetSubsystem<UWjWorldStatsSubsystem>())
		{
			StatsSub->OnUserStatsReceived.RemoveDynamic(this, &UPlayerProfileWidget::OnUserStatsReceived);
		}
	}

	DestroyPreviewActor();

	Super::NativeDestruct();
}

void UPlayerProfileWidget::ShowLocalProfile()
{
	// 플레이어 이름
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->PlayerState && PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PC->PlayerState->GetPlayerName()));
	}

	// 코스메틱 로드아웃
	FCosmeticLoadout Loadout;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>())
		{
			Loadout = CosmeticSub->GetLoadout();
		}
	}

	CreatePreviewActor(Loadout);
	PopulateLocalStats();

	SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogWjWorld, Log, TEXT("PlayerProfileWidget: ShowLocalProfile"));
}

void UPlayerProfileWidget::ShowPlayerProfile(const FUniqueNetIdRepl& PlayerId,
	const FText& PlayerName,
	const FCosmeticLoadout& Loadout)
{
	// 플레이어 이름
	if (PlayerNameText)
	{
		PlayerNameText->SetText(PlayerName);
	}

	// 3D 프리뷰
	CreatePreviewActor(Loadout);

	// 스탯 조회
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		SetVisibility(ESlateVisibility::Visible);
		return;
	}

	UWjWorldStatsSubsystem* StatsSub = GI->GetSubsystem<UWjWorldStatsSubsystem>();
	if (!StatsSub)
	{
		SetVisibility(ESlateVisibility::Visible);
		return;
	}

	FString PlayerIdStr = PlayerId.ToString();
	if (!PlayerId.IsValid() || PlayerIdStr.IsEmpty())
	{
		// UniqueId가 없는 경우 (LAN/NULL 서브시스템) — 스탯 표시 불가
		if (StatsContainer)
		{
			StatsContainer->ClearChildren();
			UTextBlock* UnavailableText = NewObject<UTextBlock>(this);
			UnavailableText->SetText(NSLOCTEXT("WjWorldProfile", "StatsUnavailable", "Stats unavailable"));
			StatsContainer->AddChild(UnavailableText);
		}
	}
	else if (StatsSub->IsUserStatsReadyByString(PlayerIdStr))
	{
		PopulateUserStats(PlayerIdStr);
	}
	else
	{
		PendingUserIdString = PlayerIdStr;
		StatsSub->OnUserStatsReceived.AddDynamic(this, &UPlayerProfileWidget::OnUserStatsReceived);
		StatsSub->RequestUserStats(PlayerId);

		// 로딩 표시
		if (StatsContainer)
		{
			StatsContainer->ClearChildren();
			UTextBlock* LoadingText = NewObject<UTextBlock>(this);
			LoadingText->SetText(NSLOCTEXT("WjWorldProfile", "Loading", "Loading stats..."));
			StatsContainer->AddChild(LoadingText);
		}
	}

	SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogWjWorld, Log, TEXT("PlayerProfileWidget: ShowPlayerProfile - %s"), *PlayerName.ToString());
}

void UPlayerProfileWidget::OnCloseClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);
	DestroyPreviewActor();
}

void UPlayerProfileWidget::OnUserStatsReceived(const FString& UserIdString)
{
	if (!PendingUserIdString.IsEmpty() && UserIdString == PendingUserIdString)
	{
		PopulateUserStats(PendingUserIdString);
		PendingUserIdString.Empty();

		// 델리게이트 제거
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UWjWorldStatsSubsystem* StatsSub = GI->GetSubsystem<UWjWorldStatsSubsystem>())
			{
				StatsSub->OnUserStatsReceived.RemoveDynamic(this, &UPlayerProfileWidget::OnUserStatsReceived);
			}
		}
	}
}

void UPlayerProfileWidget::PopulateLocalStats()
{
	if (!StatsContainer)
	{
		return;
	}

	StatsContainer->ClearChildren();

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldStatsSubsystem* StatsSub = GI->GetSubsystem<UWjWorldStatsSubsystem>();
	if (!StatsSub)
	{
		return;
	}

	const TArray<FMinigameStatDescriptor>& Descriptors = GetAllMinigameDescriptors();
	for (const FMinigameStatDescriptor& Desc : Descriptors)
	{
		// 미니게임 이름 헤더
		UTextBlock* HeaderText = NewObject<UTextBlock>(this);
		HeaderText->SetText(FText::Format(
			NSLOCTEXT("WjWorldProfile", "SectionHeader", "━━ {0} ━━"),
			Desc.MinigameName));
		FSlateFontInfo HeaderFont = HeaderText->GetFont();
		HeaderFont.Size = 14;
		HeaderText->SetFont(HeaderFont);
		StatsContainer->AddChild(HeaderText);

		// 스탯 값들
		FString StatsLine;
		for (int32 i = 0; i < Desc.Stats.Num(); ++i)
		{
			const FMinigameStatEntry& Entry = Desc.Stats[i];
			int32 Value = StatsSub->GetLocalStat(Entry.StatName);

			if (i > 0)
			{
				StatsLine += TEXT("  |  ");
			}
			StatsLine += FString::Printf(TEXT("%s: %d"), *Entry.DisplayName.ToString(), Value);
		}

		UTextBlock* StatsText = NewObject<UTextBlock>(this);
		StatsText->SetText(FText::FromString(StatsLine));
		StatsContainer->AddChild(StatsText);
	}
}

void UPlayerProfileWidget::PopulateUserStats(const FString& UserIdString)
{
	if (!StatsContainer)
	{
		return;
	}

	StatsContainer->ClearChildren();

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldStatsSubsystem* StatsSub = GI->GetSubsystem<UWjWorldStatsSubsystem>();
	if (!StatsSub)
	{
		return;
	}

	const TArray<FMinigameStatDescriptor>& Descriptors = GetAllMinigameDescriptors();
	for (const FMinigameStatDescriptor& Desc : Descriptors)
	{
		// 미니게임 이름 헤더
		UTextBlock* HeaderText = NewObject<UTextBlock>(this);
		HeaderText->SetText(FText::Format(
			NSLOCTEXT("WjWorldProfile", "SectionHeader", "━━ {0} ━━"),
			Desc.MinigameName));
		FSlateFontInfo HeaderFont = HeaderText->GetFont();
		HeaderFont.Size = 14;
		HeaderText->SetFont(HeaderFont);
		StatsContainer->AddChild(HeaderText);

		// 스탯 값들
		FString StatsLine;
		for (int32 i = 0; i < Desc.Stats.Num(); ++i)
		{
			const FMinigameStatEntry& Entry = Desc.Stats[i];
			int32 Value = StatsSub->GetUserStatByString(UserIdString, Entry.StatName);

			if (i > 0)
			{
				StatsLine += TEXT("  |  ");
			}
			StatsLine += FString::Printf(TEXT("%s: %d"), *Entry.DisplayName.ToString(), Value);
		}

		UTextBlock* StatsText = NewObject<UTextBlock>(this);
		StatsText->SetText(FText::FromString(StatsLine));
		StatsContainer->AddChild(StatsText);
	}
}

void UPlayerProfileWidget::CreatePreviewActor(const FCosmeticLoadout& Loadout)
{
	DestroyPreviewActor();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 월드에서 보이지 않는 위치에 스폰
	FVector SpawnLocation(0.f, 0.f, -10000.f);
	PreviewActor = World->SpawnActor<ACharacterPreviewActor>(ACharacterPreviewActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (PreviewActor)
	{
		// 기본 캐릭터 메시 설정 (모든 캐릭터가 같은 base mesh 사용)
		APlayerController* PC = GetOwningPlayer();
		if (PC && PC->GetPawn())
		{
			PreviewActor->SetupFromPawn(PC->GetPawn());
		}

		PreviewActor->SetupPreview(Loadout);

		// 실시간 캡처이므로 즉시 렌더 타겟 적용 (코스메틱 로드되는 대로 자동 반영)
		ApplyRenderTargetToImage();
	}
}

void UPlayerProfileWidget::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UPlayerProfileWidget::ApplyRenderTargetToImage()
{
	if (!CharacterPreviewImage || !PreviewActor)
	{
		return;
	}

	UTextureRenderTarget2D* RT = PreviewActor->GetRenderTarget();
	if (!RT)
	{
		return;
	}

	// RenderTarget을 브러시로 설정
	CharacterPreviewImage->SetBrushResourceObject(RT);

	UE_LOG(LogWjWorld, Log, TEXT("PlayerProfileWidget: RenderTarget applied to image"));
}
