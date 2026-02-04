// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Local/Lobby/WjWorldGameModeLobby.h"
#include "Core/Local/Lobby/WjWorldGameStateLobby.h"
#include "Core/Local/Lobby/WjWorldHUDLobby.h"
#include "Core/Local/Lobby/WjWorldPlayerControllerLobby.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Save/WjWorldLayoutSaveGame.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "UI/Session/CreateRoomWindow.h"
#include "UI/Session/RoomListWindow.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

AWjWorldGameModeLobby::AWjWorldGameModeLobby()
{
	PrimaryActorTick.bCanEverTick = false;

	// 기본 HUD 클래스 설정
	HUDClass = AWjWorldHUDLobby::StaticClass();

	// PlayerController 클래스 설정
	PlayerControllerClass = AWjWorldPlayerControllerLobby::StaticClass();

	// GameState 클래스 설정 (멀티플레이 배치 동기화용)
	GameStateClass = AWjWorldGameStateLobby::StaticClass();

	// DefaultPawnClass, CreateRoomWindowClass, RoomListWindowClass는 BP 서브클래스에서 설정
}

void AWjWorldGameModeLobby::BeginPlay()
{
	Super::BeginPlay();

	// 카탈로그 로드
	LoadCatalog();

	// 서버(호스트)에서만 레이아웃 로드
	if (HasAuthority())
	{
		LoadHostLayoutToGameState();
	}

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: BeginPlay - Lobby loaded (Authority: %s)"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void AWjWorldGameModeLobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	InitializeNewPlayer(NewPlayer);

	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: Player logged in - %s"),
		*NewPlayer->GetName());
}

void AWjWorldGameModeLobby::Logout(AController* Exiting)
{
	UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: Player logging out - %s"),
		Exiting ? *Exiting->GetName() : TEXT("Unknown"));

	Super::Logout(Exiting);
}

void AWjWorldGameModeLobby::LoadCatalog()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings || Settings->PlaceableObjectCatalog.IsNull())
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeLobby: No PlaceableObjectCatalog configured"));
		return;
	}

	CachedCatalog = Settings->PlaceableObjectCatalog.LoadSynchronous();
	if (!CachedCatalog)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("WjWorldGameModeLobby: Failed to load PlaceableObjectCatalog"));
		return;
	}

	// GameState에도 카탈로그 설정
	AWjWorldGameStateLobby* LobbyGameState = GetLobbyGameState();
	if (LobbyGameState)
	{
		LobbyGameState->SetCatalog(CachedCatalog);
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeLobby: Catalog loaded"));
}

void AWjWorldGameModeLobby::LoadHostLayoutToGameState()
{
	if (!HasAuthority())
	{
		return;
	}

	AWjWorldGameStateLobby* LobbyGameState = GetLobbyGameState();
	if (!LobbyGameState)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("WjWorldGameModeLobby: GameStateLobby not found"));
		return;
	}

	// 호스트의 로컬 SaveGame 로드
	UWjWorldLayoutSaveGame* SaveGame = Cast<UWjWorldLayoutSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("LobbyLayout"), 0)
	);

	if (SaveGame && SaveGame->PlacedObjects.Num() > 0)
	{
		LobbyGameState->SetPlacedObjects(SaveGame->PlacedObjects);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeLobby: Host layout loaded to GameState (%d objects)"),
			SaveGame->PlacedObjects.Num());
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeLobby: No saved layout found for host"));
	}
}

void AWjWorldGameModeLobby::InitializeNewPlayer(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	// 마우스 커서 표시
	NewPlayer->bShowMouseCursor = true;
	NewPlayer->SetInputMode(FInputModeGameAndUI());

	// PlacementComponent에 카탈로그 설정
	AWjWorldPlayerControllerLobby* LobbyPC = Cast<AWjWorldPlayerControllerLobby>(NewPlayer);
	if (LobbyPC)
	{
		UWjWorldPlacementComponent* PlacementComp = LobbyPC->GetPlacementComponent();
		if (PlacementComp && CachedCatalog)
		{
			PlacementComp->SetCatalog(CachedCatalog);
		}
	}
}

AWjWorldGameStateLobby* AWjWorldGameModeLobby::GetLobbyGameState() const
{
	return GetGameState<AWjWorldGameStateLobby>();
}

bool AWjWorldGameModeLobby::IsHost(APlayerController* PC) const
{
	if (!PC)
	{
		return false;
	}

	// Listen Server에서 호스트는 로컬 플레이어
	return PC->IsLocalController() && HasAuthority();
}

void AWjWorldGameModeLobby::EnterPlacementMode()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AWjWorldPlayerControllerLobby* LobbyPC = Cast<AWjWorldPlayerControllerLobby>(PC);
	if (!LobbyPC)
	{
		return;
	}

	// 호스트만 배치 모드 진입 가능
	if (!IsHost(LobbyPC))
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("WjWorldGameModeLobby: Only host can enter placement mode"));
		return;
	}

	UWjWorldPlacementComponent* PlacementComp = LobbyPC->GetPlacementComponent();
	if (!PlacementComp)
	{
		return;
	}

	PlacementComp->EnterPlacementMode();

	// HUD 전환
	AWjWorldHUDLobby* LobbyHUD = Cast<AWjWorldHUDLobby>(LobbyPC->GetHUD());
	if (LobbyHUD)
	{
		LobbyHUD->ShowPlacementHUD(PlacementComp);
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeLobby: Entered placement mode (Host)"));
}

void AWjWorldGameModeLobby::ExitPlacementMode()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AWjWorldPlayerControllerLobby* LobbyPC = Cast<AWjWorldPlayerControllerLobby>(PC);
	if (!LobbyPC)
	{
		return;
	}

	UWjWorldPlacementComponent* PlacementComp = LobbyPC->GetPlacementComponent();
	if (PlacementComp)
	{
		PlacementComp->ExitPlacementMode();
	}

	// HUD 복원
	AWjWorldHUDLobby* LobbyHUD = Cast<AWjWorldHUDLobby>(LobbyPC->GetHUD());
	if (LobbyHUD)
	{
		LobbyHUD->HidePlacementHUD();
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("WjWorldGameModeLobby: Exited placement mode"));
}

void AWjWorldGameModeLobby::ShowCreateRoomWindow()
{
	if (!CreateRoomWindowClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: CreateRoomWindowClass is not set!"));
		return;
	}

	// 이미 인스턴스가 있으면 재사용
	if (CreateRoomWindowInstance && CreateRoomWindowInstance->IsInViewport())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeLobby: CreateRoomWindow is already open"));
		return;
	}

	// 새 인스턴스 생성
	CreateRoomWindowInstance = CreateWidget<UCreateRoomWindow>(GetWorld(), CreateRoomWindowClass);
	
	if (CreateRoomWindowInstance)
	{
		CreateRoomWindowInstance->ShowPopup();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: CreateRoomWindow opened"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: Failed to create CreateRoomWindow widget"));
	}
}

void AWjWorldGameModeLobby::ShowRoomListWindow()
{
	if (!RoomListWindowClass)
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: RoomListWindowClass is not set!"));
		return;
	}

	// 이미 인스턴스가 있으면 재사용
	if (RoomListWindowInstance && RoomListWindowInstance->IsInViewport())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("WjWorldGameModeLobby: RoomListWindow is already open"));
		return;
	}

	// 새 인스턴스 생성
	RoomListWindowInstance = CreateWidget<URoomListWindow>(GetWorld(), RoomListWindowClass);
	
	if (RoomListWindowInstance)
	{
		RoomListWindowInstance->ShowPopup();
		UE_LOG(LogWjWorld, Log, TEXT("WjWorldGameModeLobby: RoomListWindow opened"));
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("WjWorldGameModeLobby: Failed to create RoomListWindow widget"));
	}
}
