// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class UJumpMapLevelEditorSubsystem;

/**
 * JumpMap 레이아웃 에디터 Slate 패널
 * - 레이아웃 저장/불러오기/삭제 UI
 * - Window 메뉴에서 접근 가능
 */
class SJumpMapLayoutPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SJumpMapLayoutPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** UI 콜백 */
	FReply OnSaveClicked();
	FReply OnClearClicked();
	FReply OnRefreshClicked();
	FReply OnLoadLayoutClicked(TSharedPtr<FString> LayoutName);

	/** 레이아웃 목록 갱신 */
	void RefreshLayoutList();

	/** 상태 텍스트 갱신 */
	FText GetStatusText() const;

	/** ListView 행 생성 */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** 서브시스템 */
	UJumpMapLevelEditorSubsystem* GetSubsystem() const;

	/** 위젯 멤버 */
	TSharedPtr<SEditableTextBox> LayoutNameInput;
	TSharedPtr<SListView<TSharedPtr<FString>>> LayoutListView;
	TArray<TSharedPtr<FString>> LayoutNames;
};
