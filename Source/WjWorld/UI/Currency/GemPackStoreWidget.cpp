// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Currency/GemPackStoreWidget.h"
#include "UI/Currency/GemPackEntryWidget.h"
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
	Super::NativeDestruct();
}

void UGemPackStoreWidget::ShowPopup()
{
	PopulatePackList();
	AddToViewport(200);

	UE_LOG(LogWjWorldCurrency, Log, TEXT("GemPackStoreWidget: Popup shown"));
}

void UGemPackStoreWidget::ClosePopup()
{
	ClearPackList();
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
