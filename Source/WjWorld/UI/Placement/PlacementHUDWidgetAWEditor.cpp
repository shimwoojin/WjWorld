// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementHUDWidgetAWEditor.h"
#include "UI/Placement/PlacementSaveDialogWidget.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "GamePlay/Placement/IWjWorldPlacementDataProvider.h"
#include "GamePlay/Wall/WjWorldWallLayoutConverter.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "WjWorldLogCategories.h"

void UPlacementHUDWidgetAWEditor::NativeConstruct()
{
	Super::NativeConstruct();

	// AW 에디터 타이틀 설정
	SetTitleText(FText::FromString(TEXT("Approaching Wall 에디터")));
	SetControlsHintText(FText::FromString(TEXT("LMB: 배치 | DEL: 삭제 | ESC: 로비로 돌아가기")));
}

void UPlacementHUDWidgetAWEditor::OnExitClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetAWEditor: Exit clicked - returning to lobby"));

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

void UPlacementHUDWidgetAWEditor::OnSaveClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetAWEditor: Save clicked - validating layout"));

	// 저장 전 레이아웃 변환 및 유효성 검사
	FText ValidationMessage;
	bIsCurrentLayoutValid = ConvertAndValidateLayout(ValidationMessage);

	if (!SaveDialogClass)
	{
		// 다이얼로그 클래스가 없으면 기본 슬롯 이름으로 바로 저장
		if (bIsCurrentLayoutValid)
		{
			FString DefaultSlotName = GetSaveSlotNameForContext(GetCurrentContext());
			ExecuteSave(DefaultSlotName);
		}
		else
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetAWEditor: Cannot save - %s"), *ValidationMessage.ToString());
		}
		return;
	}

	// 이미 열려있으면 스킵
	if (SaveDialogInstance && SaveDialogInstance->IsInViewport())
	{
		return;
	}

	// 다이얼로그 생성
	SaveDialogInstance = CreateWidget<UPlacementSaveDialogWidget>(GetOwningPlayer(), SaveDialogClass);
	if (SaveDialogInstance)
	{
		// 현재 컨텍스트의 기본 슬롯 이름 설정
		FString DefaultSlotName = GetSaveSlotNameForContext(GetCurrentContext());
		SaveDialogInstance->SetDefaultSlotName(DefaultSlotName);
		SaveDialogInstance->SetContext(GetCurrentContext());

		// 유효성 검사 결과 표시
		SaveDialogInstance->SetValidationResult(bIsCurrentLayoutValid, ValidationMessage);

		// 콜백 바인딩 (부모 클래스의 OnSaveConfirmed가 ExecuteSave 호출)
		SaveDialogInstance->OnSaveConfirmed.AddDynamic(this, &UPlacementHUDWidgetBase::OnSaveConfirmed);
		SaveDialogInstance->ShowPopup();
	}
}

void UPlacementHUDWidgetAWEditor::ExecuteSave(const FString& SlotName)
{
	if (!bIsCurrentLayoutValid)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetAWEditor: Cannot save invalid layout"));
		return;
	}

	// CSV 파일 저장 경로 생성
	// Content/WallLayouts/ 디렉토리에 저장
	FString FileName = SlotName + TEXT(".csv");
	FString SaveDirectory = FPaths::ProjectContentDir() / TEXT("WallLayouts");
	FString FilePath = SaveDirectory / FileName;

	// 디렉토리 생성 (없으면)
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*SaveDirectory))
	{
		FileManager.MakeDirectory(*SaveDirectory, true);
	}

	// CSV 파일로 저장
	if (FWjWorldWallLayoutConverter::SaveWallLayoutToFile(CachedWallLayout, FilePath))
	{
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetAWEditor: Wall layout saved to %s (%dx%d)"),
			*FilePath, CachedRowNum, CachedColumnNum);
	}
	else
	{
		UE_LOG(LogWjWorldPlacement, Error, TEXT("PlacementHUDWidgetAWEditor: Failed to save wall layout to %s"), *FilePath);
	}

	// 기본 저장 로직도 호출 (SaveGame 저장)
	Super::ExecuteSave(SlotName);
}

bool UPlacementHUDWidgetAWEditor::ConvertAndValidateLayout(FText& OutMessage)
{
	CachedWallLayout.Empty();
	CachedRowNum = 0;
	CachedColumnNum = 0;
	CachedCenterOffset = FVector::ZeroVector;

	// PlacementComponent에서 데이터 프로바이더 가져오기
	if (!PlacementComponent)
	{
		OutMessage = FText::FromString(TEXT("PlacementComponent가 없습니다."));
		return false;
	}

	IWjWorldPlacementDataProvider* DataProvider = PlacementComponent->GetPlacementDataProvider();
	if (!DataProvider)
	{
		OutMessage = FText::FromString(TEXT("배치 데이터 프로바이더가 없습니다."));
		return false;
	}

	const TArray<FPlacedObjectSaveEntry>& PlacedObjects = DataProvider->GetPlacedObjects();

	if (PlacedObjects.Num() == 0)
	{
		OutMessage = FText::FromString(TEXT("배치된 오브젝트가 없습니다."));
		return false;
	}

	// 배치된 오브젝트들을 WallLayout으로 변환
	bool bConvertSuccess = FWjWorldWallLayoutConverter::ConvertPlacedObjectsToWallLayout(
		PlacedObjects,
		BrickSize,
		CachedWallLayout,
		CachedRowNum,
		CachedColumnNum,
		CachedCenterOffset
	);

	if (!bConvertSuccess)
	{
		OutMessage = FText::FromString(TEXT("레이아웃 변환에 실패했습니다."));
		return false;
	}

	// 유효성 검사 (IsWallClosed 등)
	bool bIsValid = FWjWorldWallLayoutConverter::ValidateWallLayout(CachedWallLayout, OutMessage);

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetAWEditor: Layout validation - Valid: %s, Message: %s"),
		bIsValid ? TEXT("Yes") : TEXT("No"), *OutMessage.ToString());

	return bIsValid;
}
