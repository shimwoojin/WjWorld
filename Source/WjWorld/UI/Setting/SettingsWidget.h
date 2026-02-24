// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WjWorldUserWidgetBase.h"
#include "SettingsWidget.generated.h"

class UComboBoxString;
class USlider;
class UTextBlock;
class UButton;

/**
 * 설정 팝업 위젯
 *
 * 기능:
 * - 디스플레이 품질: Low / Medium / High / Epic (UGameUserSettings)
 * - 마스터 볼륨: 슬라이더 0.0~1.0 (GConfig 영속)
 * - 즉시 적용 (Apply 버튼 없음)
 */
UCLASS()
class WJWORLD_API USettingsWidget : public UWjWorldUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 팝업 표시 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPopup();

	/** 팝업 닫기 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePopup();

	/** 저장된 마스터 볼륨을 오디오 디바이스에 적용 (GameInstance::Init에서도 호출) */
	static void ApplySavedMasterVolume();

protected:
	//~ UI 위젯 바인딩

	/** 그래픽 품질 ComboBox (Low / Medium / High / Epic) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> GraphicsQualityComboBox;

	/** 마스터 볼륨 슬라이더 (0.0 ~ 1.0) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MasterVolumeSlider;

	/** 볼륨 퍼센트 텍스트 ("80%" 등) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VolumePercentText;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

protected:
	//~ 콜백

	UFUNCTION()
	void OnGraphicsQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnCloseClicked();

private:
	/** 현재 설정값으로 UI 초기화 */
	void InitializeFromCurrentSettings();

	/** 볼륨 퍼센트 텍스트 업데이트 */
	void UpdateVolumePercentText(float Volume);

	/** 마스터 볼륨을 GConfig에 저장하고 적용 */
	void SaveAndApplyMasterVolume(float Volume);
};
