// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementHUDWidgetJumpMapEditor.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UPlacementHUDWidgetJumpMapEditor::NativeConstruct()
{
	Super::NativeConstruct();

	// JumpMap 에디터 타이틀 설정
	SetTitleText(FText::FromString(TEXT("JumpMap 에디터")));
	SetControlsHintText(FText::FromString(TEXT("LMB: 배치 | R: 회전 | T: 축 전환 | G: 각도 전환 | DEL: 삭제 | F: 공중모드 | ESC: 로비로 돌아가기")));
}

void UPlacementHUDWidgetJumpMapEditor::ExecuteSave(const FString& SlotName)
{
	if (PlacementComponent)
	{
		FString ErrorMessage;
		if (!PlacementComponent->ValidateJumpMapLayout(ErrorMessage))
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("JumpMapEditor: Layout validation failed - %s"), *ErrorMessage);
			// 경고만 출력하고 저장은 진행 (작업 중 세이브 허용)
		}
	}

	Super::ExecuteSave(SlotName);
}

void UPlacementHUDWidgetJumpMapEditor::OnExitClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetJumpMapEditor: Exit clicked - returning to lobby"));

	// 배치 모드 종료
	if (PlacementComponent)
	{
		PlacementComponent->ExitPlacementMode();
	}

	// 로비로 복귀
	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (Settings)
	{
		FString LobbyPath = Settings->GetLobbyMapPath();
		if (!LobbyPath.IsEmpty())
		{
			UGameplayStatics::OpenLevel(this, *LobbyPath);
		}
	}
}
