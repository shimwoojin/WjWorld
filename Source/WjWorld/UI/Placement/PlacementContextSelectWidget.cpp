// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementContextSelectWidget.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

void UPlacementContextSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LobbyButton)
	{
		LobbyButton->OnClicked.AddDynamic(this, &UPlacementContextSelectWidget::OnLobbyClicked);
	}

	if (AWEditorButton)
	{
		AWEditorButton->OnClicked.AddDynamic(this, &UPlacementContextSelectWidget::OnAWEditorClicked);
	}

	if (JumpMapEditorButton)
	{
		JumpMapEditorButton->OnClicked.AddDynamic(this, &UPlacementContextSelectWidget::OnJumpMapEditorClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UPlacementContextSelectWidget::OnCloseClicked);
	}

	// 타이틀 설정
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("배치 모드 선택")));
	}

	// 설명 텍스트 설정
	if (LobbyDescText)
	{
		LobbyDescText->SetText(FText::FromString(TEXT("로비 데코레이션 배치")));
	}
	if (AWDescText)
	{
		AWDescText->SetText(FText::FromString(TEXT("Approaching Wall 벽 레이아웃")));
	}
	if (JumpMapDescText)
	{
		JumpMapDescText->SetText(FText::FromString(TEXT("JumpMap 플랫폼/장애물")));
	}

	UpdateButtonStates();
}

void UPlacementContextSelectWidget::ShowPopup()
{
	UpdateButtonStates();
	AddToViewport(100);
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementContextSelectWidget: Popup shown"));
}

void UPlacementContextSelectWidget::ClosePopup()
{
	RemoveFromParent();
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementContextSelectWidget: Popup closed"));
}

void UPlacementContextSelectWidget::OnLobbyClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementContextSelectWidget: Lobby context selected"));
	OnContextSelected.Broadcast(EPlacementContext::Lobby);
	ClosePopup();
}

void UPlacementContextSelectWidget::OnAWEditorClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementContextSelectWidget: ApproachingWall context selected"));
	OnContextSelected.Broadcast(EPlacementContext::ApproachingWall);
	ClosePopup();
}

void UPlacementContextSelectWidget::OnJumpMapEditorClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementContextSelectWidget: JumpMap context selected"));
	OnContextSelected.Broadcast(EPlacementContext::JumpMap);
	ClosePopup();
}

void UPlacementContextSelectWidget::OnCloseClicked()
{
	ClosePopup();
}

void UPlacementContextSelectWidget::UpdateButtonStates()
{
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	// 로비는 항상 활성화
	if (LobbyButton)
	{
		LobbyButton->SetIsEnabled(true);
	}

	// AW 에디터 맵이 설정되어 있는 경우에만 활성화
	if (AWEditorButton)
	{
		bool bHasAWMap = Settings->HasEditorMapForContext(EPlacementContext::ApproachingWall);
		AWEditorButton->SetIsEnabled(bHasAWMap);

		if (AWDescText && !bHasAWMap)
		{
			AWDescText->SetText(FText::FromString(TEXT("(에디터 맵 미설정)")));
		}
	}

	// JumpMap 에디터 맵이 설정되어 있는 경우에만 활성화
	if (JumpMapEditorButton)
	{
		bool bHasJumpMap = Settings->HasEditorMapForContext(EPlacementContext::JumpMap);
		JumpMapEditorButton->SetIsEnabled(bHasJumpMap);

		if (JumpMapDescText && !bHasJumpMap)
		{
			JumpMapDescText->SetText(FText::FromString(TEXT("(에디터 맵 미설정)")));
		}
	}
}
