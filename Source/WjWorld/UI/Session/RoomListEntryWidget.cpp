// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Session/RoomListEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
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
	UE_LOG(LogWjWorld, Log, TEXT("RoomListEntryWidget: Join button clicked - Room: %s"), *CurrentRoomInfo.RoomName);

	// GameInstance를 통해 방 참가
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
	// 방 이름
	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(CurrentRoomInfo.RoomName));
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

	// 방이 가득 찼으면 입장 버튼 비활성화
	if (JoinButton)
	{
		bool bCanJoin = (CurrentRoomInfo.CurrentPlayers < CurrentRoomInfo.MaxPlayers);
		JoinButton->SetIsEnabled(bCanJoin);
	}
}
