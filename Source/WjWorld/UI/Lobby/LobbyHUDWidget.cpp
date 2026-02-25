// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Lobby/LobbyHUDWidget.h"
#include "UI/Profile/PlayerProfileWidget.h"
#include "UI/Cosmetic/CosmeticMainWindow.h"
#include "UI/Placement/PlacementContextSelectWidget.h"
#include "UI/Setting/SettingsWidget.h"
#include "Components/Button.h"
#include "Core/Local/Lobby/WjWorldGameModeLobby.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void ULobbyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnCreateRoomClicked);
	}

	if (FindRoomButton)
	{
		FindRoomButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnFindRoomClicked);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnSettingsClicked);
	}

	// 프로필 버튼
	if (ProfileButton)
	{
		ProfileButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnProfileClicked);
	}

	// 배치 모드 버튼
	if (PlacementModeButton)
	{
		PlacementModeButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnPlacementModeClicked);
	}

	// 코스메틱 버튼
	if (CosmeticButton)
	{
		CosmeticButton->OnClicked.AddDynamic(this, &ULobbyHUDWidget::OnCosmeticClicked);
	}

	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: NativeConstruct completed"));
}

void ULobbyHUDWidget::OnCreateRoomClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Create Room button clicked"));

	AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ShowCreateRoomWindow();
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: Failed to get WjWorldGameModeLobby"));
	}
}

void ULobbyHUDWidget::OnFindRoomClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Find Room button clicked"));

	AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ShowRoomListWindow();
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: Failed to get WjWorldGameModeLobby"));
	}
}

void ULobbyHUDWidget::OnSettingsClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Settings button clicked"));

	if (!SettingsWidgetClass)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("LobbyHUDWidget: SettingsWidgetClass is not set"));
		return;
	}

	// 이미 열려있으면 닫기
	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport())
	{
		SettingsWidgetInstance->ClosePopup();
		return;
	}

	// 인스턴스가 없으면 생성
	if (!SettingsWidgetInstance)
	{
		SettingsWidgetInstance = CreateWidget<USettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
	}

	if (SettingsWidgetInstance)
	{
		SettingsWidgetInstance->ShowPopup();
	}
}

void ULobbyHUDWidget::OnProfileClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Profile button clicked"));

	if (!ProfileWidgetClass)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("LobbyHUDWidget: ProfileWidgetClass is not set"));
		return;
	}

	// 이미 열려있으면 닫기
	if (ProfileWidgetInstance && ProfileWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		ProfileWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 인스턴스가 없으면 생성
	if (!ProfileWidgetInstance)
	{
		ProfileWidgetInstance = CreateWidget<UPlayerProfileWidget>(GetOwningPlayer(), ProfileWidgetClass);
		if (ProfileWidgetInstance)
		{
			ProfileWidgetInstance->AddToViewport(100);
		}
	}

	if (ProfileWidgetInstance)
	{
		ProfileWidgetInstance->ShowLocalProfile();
	}
}

void ULobbyHUDWidget::OnPlacementModeClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Placement Mode button clicked"));

	// 컨텍스트 선택 팝업 표시
	if (!PlacementContextSelectClass)
	{
		// 클래스가 설정되지 않은 경우 직접 로비 배치 모드로 진입 (하위 호환성)
		UE_LOG(LogWjWorld, Warning, TEXT("LobbyHUDWidget: PlacementContextSelectClass not set, entering Lobby placement mode directly"));
		OnPlacementContextSelected(EPlacementContext::Lobby);
		return;
	}

	// 이미 열려있으면 스킵
	if (PlacementContextSelectInstance && PlacementContextSelectInstance->IsInViewport())
	{
		return;
	}

	// 인스턴스 생성
	PlacementContextSelectInstance = CreateWidget<UPlacementContextSelectWidget>(GetOwningPlayer(), PlacementContextSelectClass);
	if (PlacementContextSelectInstance)
	{
		// 선택 콜백 바인딩
		PlacementContextSelectInstance->OnContextSelected.AddDynamic(this, &ULobbyHUDWidget::OnPlacementContextSelected);
		PlacementContextSelectInstance->ShowPopup();
	}
}

void ULobbyHUDWidget::OnPlacementContextSelected(EPlacementContext SelectedContext)
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Placement context selected: %s"), *GetPlacementContextName(SelectedContext));

	switch (SelectedContext)
	{
	case EPlacementContext::Lobby:
		{
			// 로비 배치 모드 진입 (현재 맵에서)
			AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
			if (GameMode)
			{
				GameMode->EnterPlacementMode();
			}
			else
			{
				UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: Failed to get WjWorldGameModeLobby"));
			}
		}
		break;

	case EPlacementContext::ApproachingWall:
	case EPlacementContext::JumpMap:
		{
			// 에디터 맵으로 이동
			const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
			if (!Settings)
			{
				UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: DeveloperSettings not found"));
				return;
			}

			FString EditorMapURL = Settings->GetEditorMapOpenLevelURL(SelectedContext);
			if (EditorMapURL.IsEmpty())
			{
				UE_LOG(LogWjWorld, Error, TEXT("LobbyHUDWidget: Editor map not configured for context %s"), *GetPlacementContextName(SelectedContext));
				return;
			}

			UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Opening editor map: %s"), *EditorMapURL);
			UGameplayStatics::OpenLevel(this, *EditorMapURL);
		}
		break;

	default:
		UE_LOG(LogWjWorld, Warning, TEXT("LobbyHUDWidget: Unknown placement context"));
		break;
	}
}

bool ULobbyHUDWidget::TryCloseTopPopup()
{
	// 우선순위: Settings → Profile → Cosmetic → PlacementContextSelect

	if (SettingsWidgetInstance && SettingsWidgetInstance->IsInViewport())
	{
		SettingsWidgetInstance->ClosePopup();
		return true;
	}

	if (ProfileWidgetInstance && ProfileWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		ProfileWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		return true;
	}

	if (CosmeticWindowInstance && CosmeticWindowInstance->IsInViewport())
	{
		CosmeticWindowInstance->ClosePopup();
		return true;
	}

	if (PlacementContextSelectInstance && PlacementContextSelectInstance->IsInViewport())
	{
		PlacementContextSelectInstance->ClosePopup();
		return true;
	}

	return false;
}

void ULobbyHUDWidget::OnCosmeticClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("LobbyHUDWidget: Cosmetic button clicked"));

	if (!CosmeticWindowClass)
	{
		UE_LOG(LogWjWorld, Warning, TEXT("LobbyHUDWidget: CosmeticWindowClass is not set"));
		return;
	}

	// 이미 열려있으면 닫기
	if (CosmeticWindowInstance && CosmeticWindowInstance->IsInViewport())
	{
		CosmeticWindowInstance->ClosePopup();
		return;
	}

	// 인스턴스가 없으면 생성
	if (!CosmeticWindowInstance)
	{
		CosmeticWindowInstance = CreateWidget<UCosmeticMainWindow>(GetOwningPlayer(), CosmeticWindowClass);
	}

	if (CosmeticWindowInstance)
	{
		CosmeticWindowInstance->ShowPopup();
	}
}
