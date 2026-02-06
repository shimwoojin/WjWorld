// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Placement/PlacementHUDWidgetBase.h"
#include "UI/Placement/PlacementSaveDialogWidget.h"
#include "UI/Placement/PlacementLoadDialogWidget.h"
#include "GamePlay/Placement/WjWorldPlacementComponent.h"
#include "DataAsset/WjWorldPlaceableObjectDataAsset.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "WjWorldLogCategories.h"

void UPlacementHUDWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnExitClicked);
	}

	if (DeleteModeButton)
	{
		DeleteModeButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnDeleteModeClicked);
	}

	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnSaveClicked);
	}

	if (LoadButton)
	{
		LoadButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnLoadClicked);
	}

	// 기본 조작 안내
	SetControlsHintText(FText::FromString(TEXT("LMB: 배치 | R: 회전 | DEL: 삭제 | ESC: 나가기")));
}

void UPlacementHUDWidgetBase::SetPlacementComponent(UWjWorldPlacementComponent* InComponent)
{
	PlacementComponent = InComponent;

	if (PlacementComponent)
	{
		PopulateCatalog(PlacementComponent->GetCatalog());
	}
}

EPlacementContext UPlacementHUDWidgetBase::GetCurrentContext() const
{
	if (PlacementComponent)
	{
		return PlacementComponent->GetCurrentContext();
	}
	return EPlacementContext::None;
}

void UPlacementHUDWidgetBase::PopulateCatalog(UWjWorldPlaceableObjectDataAsset* Catalog)
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
		ItemButton->OnClicked.AddDynamic(this, &UPlacementHUDWidgetBase::OnCatalogButtonClicked);

		CatalogScrollBox->AddChild(ItemButton);
	}

	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Catalog populated with %d items"), Catalog->Objects.Num());
}

void UPlacementHUDWidgetBase::OnExitClicked()
{
	// 서브클래스에서 오버라이드
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Exit clicked (base implementation)"));
}

void UPlacementHUDWidgetBase::OnDeleteModeClicked()
{
	if (PlacementComponent)
	{
		PlacementComponent->ToggleDeleteMode();
	}
}

void UPlacementHUDWidgetBase::OnSaveClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Save clicked"));

	if (!SaveDialogClass)
	{
		// 다이얼로그 클래스가 없으면 기본 슬롯 이름으로 바로 저장
		FString DefaultSlotName = GetCurrentLoadedSlotName();
		if (DefaultSlotName.IsEmpty())
		{
			DefaultSlotName = GetSaveSlotNameForContext(GetCurrentContext());
		}
		ExecuteSave(DefaultSlotName);
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
		// 로드된 슬롯 이름이 있으면 그것을 기본값으로, 없으면 컨텍스트 기본값 사용
		FString DefaultSlotName = GetCurrentLoadedSlotName();
		if (DefaultSlotName.IsEmpty())
		{
			DefaultSlotName = GetSaveSlotNameForContext(GetCurrentContext());
		}
		SaveDialogInstance->SetDefaultSlotName(DefaultSlotName);
		SaveDialogInstance->SetContext(GetCurrentContext());

		// 콜백 바인딩
		SaveDialogInstance->OnSaveConfirmed.AddDynamic(this, &UPlacementHUDWidgetBase::OnSaveConfirmed);
		SaveDialogInstance->ShowPopup();
	}
}

void UPlacementHUDWidgetBase::OnSaveConfirmed(const FString& SlotName)
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Save confirmed with slot name: %s"), *SlotName);
	ExecuteSave(SlotName);
}

void UPlacementHUDWidgetBase::ExecuteSave(const FString& SlotName)
{
	// 기본 구현: PlacementComponent의 SaveLayoutToSlot 호출
	if (PlacementComponent)
	{
		PlacementComponent->SaveLayoutToSlot(SlotName);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Layout saved to slot '%s'"), *SlotName);
	}
}

void UPlacementHUDWidgetBase::OnCatalogButtonClicked()
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

void UPlacementHUDWidgetBase::OnCatalogItemClicked(FName ObjectId)
{
	if (PlacementComponent)
	{
		PlacementComponent->SelectObject(ObjectId);
		UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Selected object %s"), *ObjectId.ToString());
	}
}

void UPlacementHUDWidgetBase::SetTitleText(const FText& Title)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}
}

void UPlacementHUDWidgetBase::SetControlsHintText(const FText& Hint)
{
	if (ControlsHintText)
	{
		ControlsHintText->SetText(Hint);
	}
}

void UPlacementHUDWidgetBase::OnLoadClicked()
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Load clicked"));

	if (!LoadDialogClass)
	{
		UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetBase: LoadDialogClass not set"));
		return;
	}

	// 이미 열려있으면 스킵
	if (LoadDialogInstance && LoadDialogInstance->IsInViewport())
	{
		return;
	}

	// 다이얼로그 생성
	LoadDialogInstance = CreateWidget<UPlacementLoadDialogWidget>(GetOwningPlayer(), LoadDialogClass);
	if (LoadDialogInstance)
	{
		// 저장된 슬롯 목록 가져오기
		TArray<FString> SlotNames;
		if (PlacementComponent)
		{
			SlotNames = PlacementComponent->GetSavedLayoutSlots();
		}

		LoadDialogInstance->SetSlotList(SlotNames);
		LoadDialogInstance->SetContext(GetCurrentContext());

		// 콜백 바인딩
		LoadDialogInstance->OnLoadConfirmed.AddDynamic(this, &UPlacementHUDWidgetBase::OnLoadConfirmed);
		LoadDialogInstance->ShowPopup();
	}
}

void UPlacementHUDWidgetBase::OnLoadConfirmed(const FString& SlotName)
{
	UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Load confirmed with slot name: %s"), *SlotName);
	ExecuteLoad(SlotName);
}

void UPlacementHUDWidgetBase::ExecuteLoad(const FString& SlotName)
{
	// 기본 구현: PlacementComponent의 LoadLayoutFromSlot 호출
	if (PlacementComponent)
	{
		if (PlacementComponent->LoadLayoutFromSlot(SlotName))
		{
			UE_LOG(LogWjWorldPlacement, Log, TEXT("PlacementHUDWidgetBase: Layout loaded from slot '%s'"), *SlotName);
		}
		else
		{
			UE_LOG(LogWjWorldPlacement, Warning, TEXT("PlacementHUDWidgetBase: Failed to load layout from slot '%s'"), *SlotName);
		}
	}
}

FString UPlacementHUDWidgetBase::GetCurrentLoadedSlotName() const
{
	if (PlacementComponent)
	{
		return PlacementComponent->GetLoadedSlotName();
	}
	return FString();
}
