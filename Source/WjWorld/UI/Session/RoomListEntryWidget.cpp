// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Session/RoomListEntryWidget.h"
#include "UI/Session/RoomListWindow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Core/WjWorldGameInstance.h"
#include "Core/Session/SessionManager.h"
#include "WjWorldLogCategories.h"

void URoomListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &URoomListEntryWidget::OnJoinClicked);
	}

	// SessionManager 델리게이트 바인딩
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetSessionManager())
	{
		GameInstance->GetSessionManager()->OnRoomJoinedEvent.AddDynamic(this, &URoomListEntryWidget::OnRoomJoined);
	}

	UE_LOG(LogWjWorld, Log, TEXT("RoomListEntryWidget: NativeConstruct completed"));
}

void URoomListEntryWidget::SetRoomInfo(const FRoomInfo& InRoomInfo)
{
	CurrentRoomInfo = InRoomInfo;
	UpdateUI();

	UE_LOG(LogWjWorld, Log, TEXT("RoomListEntryWidget: Room info set - %s"), *InRoomInfo.RoomName);
}

void URoomListEntryWidget::OnJoinClicked()
{
	UE_LOG(LogWjWorld, Log, TEXT("RoomListEntryWidget: Join button clicked - Room: %s (Private: %d)"),
		*CurrentRoomInfo.RoomName, CurrentRoomInfo.bIsPrivate);

	// 비공개 방이면 비밀번호 입력 팝업을 부모 RoomListWindow에 요청
	if (CurrentRoomInfo.bIsPrivate)
	{
		// ScrollBox → RoomListWindow 계층으로 부모 탐색
		URoomListWindow* RoomListWindow = nullptr;
		UWidget* Parent = GetParent();
		while (Parent)
		{
			// ScrollBox의 Outer를 통해 RoomListWindow 찾기
			RoomListWindow = Cast<URoomListWindow>(Parent->GetTypedOuter<URoomListWindow>());
			if (RoomListWindow)
			{
				break;
			}
			Parent = Parent->GetParent();
		}

		if (RoomListWindow)
		{
			RoomListWindow->RequestJoinPrivateRoom(CurrentRoomInfo.SearchResultIndex);
			return;
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("RoomListEntryWidget: Could not find parent RoomListWindow"));
		}
	}

	// 공개 방 → 바로 입장
	UWjWorldGameInstance* GameInstance = Cast<UWjWorldGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		bool bSuccess = GameInstance->JoinRoom(CurrentRoomInfo.SearchResultIndex);
		if (bSuccess)
		{
			UE_LOG(LogWjWorld, Log, TEXT("RoomListEntryWidget: Room join initiated"));

			// 버튼 비활성화 (중복 클릭 방지)
			if (JoinButton)
			{
				JoinButton->SetIsEnabled(false);
			}
		}
		else
		{
			UE_LOG(LogWjWorld, Error, TEXT("RoomListEntryWidget: Failed to initiate room join"));
		}
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListEntryWidget: GameInstance is null"));
	}
}

void URoomListEntryWidget::OnRoomJoined(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogWjWorld, Log, TEXT("RoomListEntryWidget: Joined room successfully"));
		// JoinSession 내부에서 자동으로 서버 맵으로 이동됨
	}
	else
	{
		UE_LOG(LogWjWorld, Error, TEXT("RoomListEntryWidget: Failed to join room"));
		
		// 버튼 다시 활성화
		if (JoinButton)
		{
			JoinButton->SetIsEnabled(true);
		}
		
		// TODO: 에러 메시지 표시
	}
}

void URoomListEntryWidget::UpdateUI()
{
	// 방 이름 (진행 중/비공개 접두사 추가)
	if (RoomNameText)
	{
		FString DisplayName = CurrentRoomInfo.RoomName;
		if (CurrentRoomInfo.bIsPrivate)
		{
			DisplayName = FString::Printf(TEXT("[Private] %s"), *DisplayName);
		}
		if (CurrentRoomInfo.bInProgress)
		{
			DisplayName = FString::Printf(TEXT("[Playing] %s"), *DisplayName);
		}
		RoomNameText->SetText(FText::FromString(DisplayName));
	}

	// 게임 모드
	if (GameModeText)
	{
		GameModeText->SetText(FText::FromString(CurrentRoomInfo.GameMode));
	}

	// 플레이어 수
	if (PlayerCountText)
	{
		FString PlayerCount = FString::Printf(TEXT("%d / %d"),
			CurrentRoomInfo.CurrentPlayers,
			CurrentRoomInfo.MaxPlayers);
		PlayerCountText->SetText(FText::FromString(PlayerCount));
	}

	// 핑
	if (PingText)
	{
		FString Ping = FString::Printf(TEXT("%dms"), CurrentRoomInfo.Ping);
		PingText->SetText(FText::FromString(Ping));
	}

	// 방이 가득 찼거나, 게임 중 입장 불허 시 버튼 비활성화
	if (JoinButton)
	{
		bool bCanJoin = (CurrentRoomInfo.CurrentPlayers < CurrentRoomInfo.MaxPlayers);
		if (CurrentRoomInfo.bInProgress && !CurrentRoomInfo.bAllowJoinInProgress)
		{
			bCanJoin = false;
		}
		JoinButton->SetIsEnabled(bCanJoin);
	}
}
