// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Cosmetic/CosmeticItemEntryWidget.h"
#include "Cosmetic/WjWorldCosmeticDataAsset.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "WjWorldLogCategories.h"

void UCosmeticItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemButton)
	{
		ItemButton->OnClicked.AddDynamic(this, &UCosmeticItemEntryWidget::OnItemButtonClicked);
	}

	// 초기 상태: 선택 테두리 숨김, 장착 아이콘 숨김
	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
	}

	if (EquippedIcon)
	{
		EquippedIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCosmeticItemEntryWidget::NativeDestruct()
{
	// 비동기 로드 취소
	if (IconLoadHandle.IsValid())
	{
		IconLoadHandle->CancelHandle();
		IconLoadHandle.Reset();
	}

	Super::NativeDestruct();
}

void UCosmeticItemEntryWidget::SetItemDefinition(const FCosmeticItemDefinition& Def)
{
	CachedItemId = Def.ItemId;
	CachedCoinPrice = Def.CoinPrice;
	CachedGemPrice = Def.GemPrice;

	// 이름 설정
	if (NameText)
	{
		NameText->SetText(Def.DisplayName);
	}

	// 희귀도 바 색상 설정
	if (RarityBar)
	{
		FLinearColor RarityColor = GetRarityColor(Def.Rarity);
		RarityBar->SetColorAndOpacity(RarityColor);
	}

	// 가격 표시 (보유 여부는 SetOwned로 갱신)
	if (PriceText)
	{
		if (Def.CoinPrice > 0)
		{
			PriceText->SetText(FText::Format(NSLOCTEXT("Cosmetic", "CoinPriceEntryFormat", "{0} Coin"), FText::AsNumber(Def.CoinPrice)));
		}
		else if (Def.GemPrice > 0)
		{
			PriceText->SetText(FText::Format(NSLOCTEXT("Cosmetic", "GemPriceEntryFormat", "{0} Gem"), FText::AsNumber(Def.GemPrice)));
		}
		else
		{
			PriceText->SetText(NSLOCTEXT("Cosmetic", "Free", "무료"));
		}
	}

	// 아이콘 비동기 로드
	if (IconImage && !Def.Icon.IsNull())
	{
		// 기존 로드 취소
		if (IconLoadHandle.IsValid())
		{
			IconLoadHandle->CancelHandle();
		}

		// 플레이스홀더 색상 (로딩 중)
		IconImage->SetColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.f));

		IconLoadHandle = StreamableManager.RequestAsyncLoad(
			Def.Icon.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &UCosmeticItemEntryWidget::OnIconLoaded)
		);
	}
}

void UCosmeticItemEntryWidget::SetOwned(bool bOwned)
{
	bIsOwned = bOwned;

	if (PriceText)
	{
		if (bIsOwned)
		{
			PriceText->SetText(NSLOCTEXT("Cosmetic", "Owned", "보유중"));
			PriceText->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.8f, 0.2f, 1.f))); // 초록색
		}
		else
		{
			// 미보유 시 캐시된 가격으로 복원
			if (CachedCoinPrice > 0)
			{
				PriceText->SetText(FText::Format(NSLOCTEXT("Cosmetic", "CoinPriceEntryFormat", "{0} Coin"), FText::AsNumber(CachedCoinPrice)));
			}
			else if (CachedGemPrice > 0)
			{
				PriceText->SetText(FText::Format(NSLOCTEXT("Cosmetic", "GemPriceEntryFormat", "{0} Gem"), FText::AsNumber(CachedGemPrice)));
			}
			else
			{
				PriceText->SetText(NSLOCTEXT("Cosmetic", "Free", "무료"));
			}
			PriceText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}
}

void UCosmeticItemEntryWidget::SetEquipped(bool bEquipped)
{
	bIsEquipped = bEquipped;

	if (EquippedIcon)
	{
		EquippedIcon->SetVisibility(bEquipped ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCosmeticItemEntryWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;

	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UCosmeticItemEntryWidget::OnItemButtonClicked()
{
	OnItemClicked.Broadcast(CachedItemId);
}

FLinearColor UCosmeticItemEntryWidget::GetRarityColor(ECosmeticRarity Rarity) const
{
	switch (Rarity)
	{
	case ECosmeticRarity::Common:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.f); // 회색
	case ECosmeticRarity::Uncommon:
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.f); // 녹색
	case ECosmeticRarity::Rare:
		return FLinearColor(0.2f, 0.4f, 1.f, 1.f);  // 파랑
	case ECosmeticRarity::Epic:
		return FLinearColor(0.6f, 0.2f, 0.8f, 1.f); // 보라
	case ECosmeticRarity::Legendary:
		return FLinearColor(1.f, 0.6f, 0.f, 1.f);   // 주황
	default:
		return FLinearColor::White;
	}
}

void UCosmeticItemEntryWidget::OnIconLoaded()
{
	if (!IconLoadHandle.IsValid() || !IconLoadHandle->HasLoadCompleted())
	{
		return;
	}

	UObject* LoadedAsset = IconLoadHandle->GetLoadedAsset();
	if (UTexture2D* IconTexture = Cast<UTexture2D>(LoadedAsset))
	{
		if (IconImage)
		{
			IconImage->SetBrushFromTexture(IconTexture);
			IconImage->SetColorAndOpacity(FLinearColor::White);
		}
	}

	IconLoadHandle.Reset();
}
