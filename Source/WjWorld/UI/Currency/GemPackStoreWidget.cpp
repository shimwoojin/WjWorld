// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Currency/GemPackStoreWidget.h"
#include "UI/Currency/GemPackEntryWidget.h"
#include "UI/Currency/GemCoinExchangeEntryWidget.h"
#include "Currency/WjWorldCurrencySubsystem.h"
#include "Setting/WjWorldDeveloperSettings.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

void UGemPackStoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UGemPackStoreWidget::OnCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UGemPackStoreWidget::OnCloseClicked);
	}
}

void UGemPackStoreWidget::NativeDestruct()
{
	ClearPackList();
	ClearExchangeList();

	// 델리게이트 해제
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
		if (CurrencySub)
		{
			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UGemPackStoreWidget::OnGemBalanceChanged);
		}
	}

	Super::NativeDestruct();
}

void UGemPackStoreWidget::ShowPopup()
{
	PopulatePackList();
	PopulateExchangeList();

	// Gem 잔액 변경 구독 (교환 버튼 상태 갱신용)
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
		if (CurrencySub)
		{
			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UGemPackStoreWidget::OnGemBalanceChanged);
			CurrencySub->OnCurrencyBalanceChanged.AddDynamic(this, &UGemPackStoreWidget::OnGemBalanceChanged);
		}
	}

	AddToViewport(200);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Popup shown"));
}

void UGemPackStoreWidget::ClosePopup()
{
	ClearPackList();
	ClearExchangeList();

	// 델리게이트 해제
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
		if (CurrencySub)
		{
			CurrencySub->OnCurrencyBalanceChanged.RemoveDynamic(this, &UGemPackStoreWidget::OnGemBalanceChanged);
		}
	}

	RemoveFromParent();

	UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Popup closed"));
}

void UGemPackStoreWidget::OnCloseClicked()
{
	ClosePopup();
}

void UGemPackStoreWidget::OnPackPurchaseClicked(int32 PackDefId)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (CurrencySub)
	{
		CurrencySub->PurchaseGemPack(PackDefId);
		UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Purchase requested for PackDefId=%d"), PackDefId);
	}
}

void UGemPackStoreWidget::PopulatePackList()
{
	ClearPackList();

	if (!PackEntryWidgetClass)
	{
		UE_LOG(LogWjWorldCurrency, Warning, TEXT("GemPackStoreWidget: PackEntryWidgetClass is not set"));
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	for (const FGemPackDefinition& PackDef : Settings->GemPackDefinitions)
	{
		UGemPackEntryWidget* EntryWidget = CreateWidget<UGemPackEntryWidget>(GetOwningPlayer(), PackEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetPackDefinition(PackDef);
		EntryWidget->OnBuyClicked.AddDynamic(this, &UGemPackStoreWidget::OnPackPurchaseClicked);

		if (PackListBox)
		{
			PackListBox->AddChild(EntryWidget);
		}

		PackEntryWidgets.Add(EntryWidget);
	}

	UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Populated %d pack entries"), PackEntryWidgets.Num());
}

void UGemPackStoreWidget::ClearPackList()
{
	for (UGemPackEntryWidget* Entry : PackEntryWidgets)
	{
		if (Entry)
		{
			Entry->OnBuyClicked.RemoveDynamic(this, &UGemPackStoreWidget::OnPackPurchaseClicked);
			Entry->RemoveFromParent();
		}
	}
	PackEntryWidgets.Empty();

	if (PackListBox)
	{
		PackListBox->ClearChildren();
	}
}

void UGemPackStoreWidget::OnExchangeClicked(int32 ExchangeDefId)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	if (CurrencySub)
	{
		CurrencySub->ExchangeGemsForCoins(ExchangeDefId);
		UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Exchange requested for DefId=%d"), ExchangeDefId);
	}
}

void UGemPackStoreWidget::OnGemBalanceChanged(ECurrencyType CurrencyType, int32 NewBalance)
{
	if (CurrencyType == ECurrencyType::Gem)
	{
		UpdateExchangeButtonStates();
	}
}

void UGemPackStoreWidget::PopulateExchangeList()
{
	ClearExchangeList();

	if (!ExchangeEntryWidgetClass || !ExchangeListBox)
	{
		// ExchangeListBox가 없으면 교환 섹션 미사용
		if (ExchangeSectionText)
		{
			ExchangeSectionText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const UWjWorldDeveloperSettings* Settings = GetDefault<UWjWorldDeveloperSettings>();
	if (!Settings || Settings->GemCoinExchangeDefinitions.Num() == 0)
	{
		if (ExchangeSectionText)
		{
			ExchangeSectionText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	if (ExchangeSectionText)
	{
		ExchangeSectionText->SetVisibility(ESlateVisibility::Visible);
	}

	for (const FGemCoinExchangeDefinition& ExchangeDef : Settings->GemCoinExchangeDefinitions)
	{
		UGemCoinExchangeEntryWidget* EntryWidget = CreateWidget<UGemCoinExchangeEntryWidget>(
			GetOwningPlayer(), ExchangeEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetExchangeDefinition(ExchangeDef);
		EntryWidget->OnExchangeClicked.AddDynamic(this, &UGemPackStoreWidget::OnExchangeClicked);

		ExchangeListBox->AddChild(EntryWidget);
		ExchangeEntryWidgets.Add(EntryWidget);
	}

	// 초기 버튼 상태 갱신
	UpdateExchangeButtonStates();

	UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Populated %d exchange entries"), ExchangeEntryWidgets.Num());
}

void UGemPackStoreWidget::ClearExchangeList()
{
	for (UGemCoinExchangeEntryWidget* Entry : ExchangeEntryWidgets)
	{
		if (Entry)
		{
			Entry->OnExchangeClicked.RemoveDynamic(this, &UGemPackStoreWidget::OnExchangeClicked);
			Entry->RemoveFromParent();
		}
	}
	ExchangeEntryWidgets.Empty();

	if (ExchangeListBox)
	{
		ExchangeListBox->ClearChildren();
	}
}

void UGemPackStoreWidget::UpdateExchangeButtonStates()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCurrencySubsystem* CurrencySub = GI->GetSubsystem<UWjWorldCurrencySubsystem>();
	int32 CurrentGemBalance = CurrencySub ? CurrencySub->GetBalance(ECurrencyType::Gem) : 0;

	for (UGemCoinExchangeEntryWidget* Entry : ExchangeEntryWidgets)
	{
		if (Entry)
		{
			Entry->UpdateButtonState(CurrentGemBalance);
		}
	}
}
