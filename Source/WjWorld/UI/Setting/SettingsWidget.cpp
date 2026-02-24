// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Setting/SettingsWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/GameUserSettings.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "WjWorldLogCategories.h"

namespace
{
	const TCHAR* GraphicsQualityNames[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
	constexpr int32 NumGraphicsQualities = UE_ARRAY_COUNT(GraphicsQualityNames);
}

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 콜백 바인딩
	if (GraphicsQualityComboBox)
	{
		GraphicsQualityComboBox->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnGraphicsQualityChanged);
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMasterVolumeChanged);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &USettingsWidget::OnCloseClicked);
	}

	// 현재 설정값으로 UI 초기화
	InitializeFromCurrentSettings();

	UE_LOG(LogWjWorld, Log, TEXT("SettingsWidget: NativeConstruct completed"));
}

void USettingsWidget::ShowPopup()
{
	AddToViewport(100);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}

	// 팝업 열 때마다 최신 설정값으로 갱신
	InitializeFromCurrentSettings();

	UE_LOG(LogWjWorld, Log, TEXT("SettingsWidget: Popup shown"));
}

void USettingsWidget::ClosePopup()
{
	RemoveFromParent();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}

	UE_LOG(LogWjWorld, Log, TEXT("SettingsWidget: Popup closed"));
}

void USettingsWidget::ApplySavedMasterVolume()
{
	float Volume = 1.0f;
	GConfig->GetFloat(TEXT("WjWorldSettings"), TEXT("MasterVolume"), Volume, GGameUserSettingsIni);

	if (FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice())
	{
		AudioDevice->SetTransientPrimaryVolume(Volume);
	}

	UE_LOG(LogWjWorld, Log, TEXT("SettingsWidget: Applied saved master volume: %.2f"), Volume);
}

void USettingsWidget::OnGraphicsQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	// 선택된 품질 이름을 레벨 인덱스로 변환
	for (int32 i = 0; i < NumGraphicsQualities; ++i)
	{
		if (SelectedItem == GraphicsQualityNames[i])
		{
			UserSettings->SetOverallScalabilityLevel(i);
			UserSettings->ApplySettings(false);
			UserSettings->SaveSettings();

			UE_LOG(LogWjWorld, Log, TEXT("SettingsWidget: Graphics quality changed to %s (%d)"), GraphicsQualityNames[i], i);
			break;
		}
	}
}

void USettingsWidget::OnMasterVolumeChanged(float Value)
{
	SaveAndApplyMasterVolume(Value);
	UpdateVolumePercentText(Value);
}

void USettingsWidget::OnCloseClicked()
{
	ClosePopup();
}

void USettingsWidget::InitializeFromCurrentSettings()
{
	// 그래픽 품질 초기화
	if (GraphicsQualityComboBox)
	{
		GraphicsQualityComboBox->ClearOptions();
		for (int32 i = 0; i < NumGraphicsQualities; ++i)
		{
			GraphicsQualityComboBox->AddOption(GraphicsQualityNames[i]);
		}

		int32 CurrentLevel = 3; // 기본값 Epic
		UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
		if (UserSettings)
		{
			CurrentLevel = UserSettings->GetOverallScalabilityLevel();
			if (CurrentLevel < 0 || CurrentLevel >= NumGraphicsQualities)
			{
				CurrentLevel = 3; // 커스텀(-1) → Epic 취급
			}
		}

		GraphicsQualityComboBox->SetSelectedOption(GraphicsQualityNames[CurrentLevel]);
	}

	// 마스터 볼륨 초기화
	float SavedVolume = 1.0f;
	GConfig->GetFloat(TEXT("WjWorldSettings"), TEXT("MasterVolume"), SavedVolume, GGameUserSettingsIni);

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(SavedVolume);
	}

	UpdateVolumePercentText(SavedVolume);
}

void USettingsWidget::UpdateVolumePercentText(float Volume)
{
	if (VolumePercentText)
	{
		int32 Percent = FMath::RoundToInt(Volume * 100.0f);
		VolumePercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
	}
}

void USettingsWidget::SaveAndApplyMasterVolume(float Volume)
{
	// GConfig에 저장
	GConfig->SetFloat(TEXT("WjWorldSettings"), TEXT("MasterVolume"), Volume, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);

	// 오디오 디바이스에 즉시 적용
	if (FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice())
	{
		AudioDevice->SetTransientPrimaryVolume(Volume);
	}
}
