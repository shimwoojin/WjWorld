// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Lobby/PlacementHUDWidget.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Core/Local/Lobby/WjWorldGameModeLobby.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UPlacementHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPlacementHUDWidget::OnExitClicked);
	}

	if (DeleteModeButton)
	{
		DeleteModeButton->OnClicked.AddDynamic(this, &UPlacementHUDWidget::OnDeleteModeClicked);
	}

	if (ControlsHintText)
	{
		ControlsHintText->SetText(FText::FromString(TEXT("LMB: 배치 | R: 회전 | DEL: 삭제 | ESC: 나가기")));
	}
}

void UPlacementHUDWidget::SetPlacementComponent(UWjWorldPlacementComponent* InComponent)
{
	PlacementComponent = InComponent;

	if (PlacementComponent)
	{
		PopulateCatalog(PlacementComponent->GetCatalog());
	}
}

void UPlacementHUDWidget::PopulateCatalog(UWjWorldPlaceableObjectDataAsset* Catalog)
{
	if (!CatalogScrollBox || !Catalog)
	{
		return;
	}

	CatalogScrollBox->ClearChildren();
	ButtonToObjectIdMap.Empty();

	for (const FPlaceableObjectDefinition& Def : Catalog->Objects)
	{
		UButton* ItemButton = NewObject<UButton>(this);
		UTextBlock* ItemText = NewObject<UTextBlock>(this);

		ItemText->SetText(Def.DisplayName.IsEmpty() ? FText::FromName(Def.ObjectId) : Def.DisplayName);

		// 버튼 스타일링
		FSlateFontInfo FontInfo = ItemText->GetFont();
		FontInfo.Size = 14;
		ItemText->SetFont(FontInfo);

		ItemButton->AddChild(ItemText);

		// 버튼 → ObjectId 매핑 등록 + 공통 핸들러 바인딩
		ButtonToObjectIdMap.Add(ItemButton, Def.ObjectId);
		ItemButton->OnClicked.AddDynamic(this, &UPlacementHUDWidget::OnCatalogButtonClicked);

		CatalogScrollBox->AddChild(ItemButton);
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidget: Catalog populated with %d items"), Catalog->Objects.Num());
}

void UPlacementHUDWidget::OnExitClicked()
{
	AWjWorldGameModeLobby* GameMode = Cast<AWjWorldGameModeLobby>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ExitPlacementMode();
	}
}

void UPlacementHUDWidget::OnDeleteModeClicked()
{
	if (PlacementComponent)
	{
		PlacementComponent->ToggleDeleteMode();
	}
}

void UPlacementHUDWidget::OnCatalogButtonClicked()
{
	// 호버 상태인 버튼을 찾아 ObjectId 매핑 조회
	for (const auto& Pair : ButtonToObjectIdMap)
	{
		if (Pair.Key && Pair.Key->IsHovered())
		{
			OnCatalogItemClicked(Pair.Value);
			return;
		}
	}
}

void UPlacementHUDWidget::OnCatalogItemClicked(FName ObjectId)
{
	if (PlacementComponent)
	{
		PlacementComponent->SelectObject(ObjectId);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidget: Selected object %s"), *ObjectId.ToString());
	}
}
