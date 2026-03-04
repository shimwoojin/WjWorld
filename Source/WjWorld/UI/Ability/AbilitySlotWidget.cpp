// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Ability/AbilitySlotWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WjWorldGameplayAbilityBase.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Base/WjWorldCharacterBase.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"

namespace
{
	FText ShortenKeyName(const FText& FullName)
	{
		static const TMap<FString, FString> ShortNames = {
			{ TEXT("Left Mouse Button"), TEXT("LMB") },
			{ TEXT("Right Mouse Button"), TEXT("RMB") },
			{ TEXT("Middle Mouse Button"), TEXT("MMB") },
			{ TEXT("Left Shift"), TEXT("Shift") },
			{ TEXT("Right Shift"), TEXT("Shift") },
			{ TEXT("Left Control"), TEXT("Ctrl") },
			{ TEXT("Right Control"), TEXT("Ctrl") },
			{ TEXT("Left Alt"), TEXT("Alt") },
			{ TEXT("Right Alt"), TEXT("Alt") },
			{ TEXT("Space Bar"), TEXT("Space") },
			{ TEXT("Caps Lock"), TEXT("Caps") },
			{ TEXT("Escape"), TEXT("ESC") },
			{ TEXT("Enter"), TEXT("Enter") },
			{ TEXT("Backspace"), TEXT("Back") },
			{ TEXT("Tab"), TEXT("Tab") },
		};

		const FString FullStr = FullName.ToString();
		if (const FString* Short = ShortNames.Find(FullStr))
		{
			return FText::FromString(*Short);
		}

		// Gamepad 등 "Gamepad ..." 접두사 축약
		if (FullStr.StartsWith(TEXT("Gamepad ")))
		{
			return FText::FromString(FullStr.Mid(8));
		}

		return FullName;
	}
}

void UAbilitySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 상태: 쿨다운 오버레이 숨김
	if (CooldownOverlay)
	{
		CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ChargeCountText)
	{
		ChargeCountText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 어빌리티 인스턴스 캐시 시도
	TryCacheAbilityInstance();
}

void UAbilitySlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 아직 캐시되지 않았으면 매 프레임 재시도 (어빌리티 비동기 로드 대비)
	if (!bAbilityCached)
	{
		TryCacheAbilityInstance();
		return;
	}

	// 캐시된 인스턴스가 유효하지 않으면 재시도
	if (!CachedAbilityInstance.IsValid())
	{
		bAbilityCached = false;
		return;
	}

	UpdateCooldownDisplay(InDeltaTime);
	UpdateChargeDisplay();
}

bool UAbilitySlotWidget::TryCacheAbilityInstance()
{
	if (!AbilityClass)
	{
		return false;
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return false;
	}

	AWjWorldCharacterPlay* CharacterPlay = Cast<AWjWorldCharacterPlay>(OwningPawn);
	if (!CharacterPlay)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = CharacterPlay->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	UWjWorldGameplayAbilityBase* AbilityInstance = FindAbilityInstance(ASC);
	if (!AbilityInstance)
	{
		return false;
	}

	CachedAbilityInstance = AbilityInstance;
	bAbilityCached = true;

	// 초기화: 아이콘, 키 바인딩 텍스트
	InitializeIcon();

	if (KeyBindText)
	{
		KeyBindText->SetText(ResolveKeyBindText());
	}

	return true;
}

UWjWorldGameplayAbilityBase* UAbilitySlotWidget::FindAbilityInstance(UAbilitySystemComponent* ASC) const
{
	if (!ASC || !AbilityClass)
	{
		return nullptr;
	}

	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = ASC->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
		{
			// InstancedPerActor인 경우 인스턴스 목록에서 가져옴
			if (Spec.GetAbilityInstances().Num() > 0)
			{
				return Cast<UWjWorldGameplayAbilityBase>(Spec.GetAbilityInstances()[0]);
			}
			// 인스턴스가 없으면 CDO 사용
			return Cast<UWjWorldGameplayAbilityBase>(Spec.Ability);
		}
	}

	return nullptr;
}

FText UAbilitySlotWidget::ResolveKeyBindText() const
{
	if (!AbilityClass)
	{
		return FText::GetEmpty();
	}

	// 1) OwningPawn에서 CharacterBase → DefaultMappingContext 가져오기
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return FText::GetEmpty();
	}

	AWjWorldCharacterBase* CharacterBase = Cast<AWjWorldCharacterBase>(OwningPawn);
	if (!CharacterBase)
	{
		return FText::GetEmpty();
	}

	UInputMappingContext* MappingContext = CharacterBase->GetDefaultMappingContext();
	if (!MappingContext)
	{
		return FText::GetEmpty();
	}

	// 2) ASC에서 이 AbilityClass의 InputID 찾기
	AWjWorldCharacterPlay* CharacterPlay = Cast<AWjWorldCharacterPlay>(CharacterBase);
	if (!CharacterPlay)
	{
		return FText::GetEmpty();
	}

	UAbilitySystemComponent* ASC = CharacterPlay->GetAbilitySystemComponent();
	if (!ASC)
	{
		return FText::GetEmpty();
	}

	int32 TargetInputID = INDEX_NONE;
	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = ASC->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
		{
			TargetInputID = Spec.InputID;
			break;
		}
	}

	if (TargetInputID == INDEX_NONE)
	{
		return FText::GetEmpty();
	}

	// 3) MappingContext에서 "IA_Ability{InputID}" 에 해당하는 키 찾기
	FString TargetActionName = FString::Printf(TEXT("IA_Ability%d"), TargetInputID);
	const TArray<FEnhancedActionKeyMapping>& Mappings = MappingContext->GetMappings();

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (!Mapping.Action)
		{
			continue;
		}

		if (Mapping.Action.GetName() == TargetActionName)
		{
			return ShortenKeyName(Mapping.Key.GetDisplayName(false));
		}
	}

	return FText::GetEmpty();
}

void UAbilitySlotWidget::UpdateCooldownDisplay(float DeltaTime)
{
	UWjWorldGameplayAbilityBase* AbilityInstance = CachedAbilityInstance.Get();
	if (!AbilityInstance)
	{
		return;
	}

	// 충전 기반 어빌리티: 충전이 남아있으면 쿨다운 미표시
	if (AbilityInstance->IsChargeBased())
	{
		if (AbilityInstance->GetCurrentCharges() > 0)
		{
			if (CooldownOverlay)
			{
				CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (CooldownText)
			{
				CooldownText->SetVisibility(ESlateVisibility::Collapsed);
			}
			return;
		}
	}

	// ASC 가져오기
	APawn* OwningPawn = GetOwningPlayerPawn();
	AWjWorldCharacterPlay* CharacterPlay = OwningPawn ? Cast<AWjWorldCharacterPlay>(OwningPawn) : nullptr;
	UAbilitySystemComponent* ASC = CharacterPlay ? CharacterPlay->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		return;
	}

	// 쿨다운 태그로 잔여/전체 시간 조회
	const FGameplayTagContainer* CooldownTags = AbilityInstance->GetCooldownTags();
	if (!CooldownTags || CooldownTags->Num() == 0)
	{
		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
	TArray<float> Remainings = ASC->GetActiveEffectsTimeRemaining(Query);
	TArray<float> FullDurations = ASC->GetActiveEffectsDuration(Query);

	float MaxRemaining = 0.f;
	float CorrespondingDuration = 0.f;
	for (int32 i = 0; i < Remainings.Num(); ++i)
	{
		if (Remainings[i] > MaxRemaining)
		{
			MaxRemaining = Remainings[i];
			CorrespondingDuration = (i < FullDurations.Num()) ? FullDurations[i] : 0.f;
		}
	}

	if (MaxRemaining > 0.f)
	{
		float Percent = (CorrespondingDuration > 0.f) ? (MaxRemaining / CorrespondingDuration) : 1.f;

		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
			CooldownOverlay->SetPercent(Percent);
		}
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (AbilityInstance->IsChargeBased())
			{
				CooldownText->SetText(FText::AsNumber(FMath::CeilToInt32(MaxRemaining)));
			}
			else
			{
				CooldownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), MaxRemaining)));
			}
		}
	}
	else
	{
		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UAbilitySlotWidget::UpdateChargeDisplay()
{
	UWjWorldGameplayAbilityBase* AbilityInstance = CachedAbilityInstance.Get();
	if (!AbilityInstance || !AbilityInstance->IsChargeBased())
	{
		if (ChargeCountText)
		{
			ChargeCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	if (ChargeCountText)
	{
		int32 CurrentCharges = AbilityInstance->GetCurrentCharges();
		int32 MaxCharges = AbilityInstance->GetMaxCharges();

		ChargeCountText->SetVisibility(ESlateVisibility::HitTestInvisible);
		ChargeCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentCharges, MaxCharges)));
	}
}

void UAbilitySlotWidget::InitializeIcon()
{
	if (!AbilityClass)
	{
		return;
	}

	const UWjWorldGameplayAbilityBase* CDO = AbilityClass.GetDefaultObject();
	if (!CDO || !CDO->AbilityIcon)
	{
		return;
	}

	if (AbilityIconImage)
	{
		AbilityIconImage->SetBrushFromTexture(CDO->AbilityIcon);
	}

	// ProgressBar FillImage에도 같은 아이콘 설정
	if (CooldownOverlay)
	{
		FProgressBarStyle Style = CooldownOverlay->GetWidgetStyle();
		Style.FillImage.SetResourceObject(CDO->AbilityIcon);
		CooldownOverlay->SetWidgetStyle(Style);
	}
}
