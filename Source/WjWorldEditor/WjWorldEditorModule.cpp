// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldEditorModule.h"
#include "JumpMap/SJumpMapLayoutPanel.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "WjWorldEditor"

const FName FWjWorldEditorModule::JumpMapLayoutTabName(TEXT("JumpMapLayoutEditor"));

void FWjWorldEditorModule::StartupModule()
{
	RegisterTabs();
}

void FWjWorldEditorModule::ShutdownModule()
{
	UnregisterTabs();
}

void FWjWorldEditorModule::RegisterTabs()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		JumpMapLayoutTabName,
		FOnSpawnTab::CreateRaw(this, &FWjWorldEditorModule::SpawnJumpMapLayoutTab))
		.SetDisplayName(LOCTEXT("JumpMapLayoutTabTitle", "JumpMap Layout Editor"))
		.SetTooltipText(LOCTEXT("JumpMapLayoutTabTooltip", "JumpMap 레이아웃 저장/불러오기 도구"));
}

void FWjWorldEditorModule::UnregisterTabs()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(JumpMapLayoutTabName);
}

TSharedRef<SDockTab> FWjWorldEditorModule::SpawnJumpMapLayoutTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SJumpMapLayoutPanel)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWjWorldEditorModule, WjWorldEditor)
