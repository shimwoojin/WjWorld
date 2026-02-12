// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FWjWorldEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterTabs();
	void UnregisterTabs();

	TSharedRef<SDockTab> SpawnJumpMapLayoutTab(const FSpawnTabArgs& SpawnTabArgs);

	static const FName JumpMapLayoutTabName;
};
