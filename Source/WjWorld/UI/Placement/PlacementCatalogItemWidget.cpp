// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementCatalogItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UPlacementCatalogItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemButton)
	{
		ItemButton->OnClicked.AddDynamic(this, &UPlacementCatalogItemWidget::HandleItemClicked);
	}
	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UPlacementCatalogItemWidget::HandleBuyClicked);
	}
}

void UPlacementCatalogItemWidget::SetItemData(FName InObjectId, const FText& InDisplayName, UTexture2D* InIcon)
{
	ObjectId = InObjectId;

	if (ItemNameText)
	{
		ItemNameText->SetText(InDisplayName);
	}

	if (ItemIcon)
	{
		if (InIcon)
		{
			ItemIcon->SetBrushFromTexture(InIcon);
			ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPlacementCatalogItemWidget::SetPlacementCount(int32 PlacedCount, int32 MaxCount)
{
	if (PlacementCountText)
	{
		if (MaxCount > 0)
		{
			PlacementCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), PlacedCount, MaxCount)));
			PlacementCountText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			PlacementCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPlacementCatalogItemWidget::SetBuyInfo(bool bShowBuy, int32 CoinPrice, bool bCanAfford)
{
	if (BuyButton)
	{
		if (bShowBuy)
		{
			BuyButton->SetVisibility(ESlateVisibility::Visible);
			BuyButton->SetIsEnabled(bCanAfford);
		}
		else
		{
			BuyButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (BuyPriceText && bShowBuy)
	{
		BuyPriceText->SetText(FText::FromString(FString::Printf(TEXT("%d Coin"), CoinPrice)));
	}
}

void UPlacementCatalogItemWidget::SetOwned(bool bOwned)
{
	if (ItemNameText)
	{
		if (bOwned)
		{
			ItemNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		else
		{
			ItemNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
		}
	}

	if (ItemIcon)
	{
		if (bOwned)
		{
			ItemIcon->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			ItemIcon->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 0.5f));
		}
	}
}

void UPlacementCatalogItemWidget::HandleItemClicked()
{
	OnItemSelected.Broadcast(ObjectId);
}

void UPlacementCatalogItemWidget::HandleBuyClicked()
{
	OnBuyClicked.Broadcast(ObjectId);
}
