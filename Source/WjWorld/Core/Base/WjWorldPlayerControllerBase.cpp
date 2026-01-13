// Fill out your copyright notice in the Description page of Project Settings.

#include "WjWorldPlayerControllerBase.h"
#include "WjWorldCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "WjWorldLogCategories.h"

AWjWorldPlayerControllerBase::AWjWorldPlayerControllerBase()
{
	
}

void AWjWorldPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeController();
	InitializeUI();
}

void AWjWorldPlayerControllerBase::InitializeController()
{
	// Base implementation - override in derived classes
}

void AWjWorldPlayerControllerBase::InitializeUI()
{
	// Base implementation - override in derived classes
}

void AWjWorldPlayerControllerBase::CheckInputMode()
{
#if UE_ENABLE_DEBUG_DRAWING
	FString InputModeString = GetCurrentInputModeDebugString();
	UE_LOG(LogWjWorld, Log, TEXT("Current Input Mode: %s"), *InputModeString);
#endif
}

void AWjWorldPlayerControllerBase::ChangeCharacterViewMode(int32 InViewMode)
{
	AWjWorldCharacterBase* WjCharacter = Cast<AWjWorldCharacterBase>(GetPawn());
	if (WjCharacter == nullptr)
	{
		return;
	}

	switch (InViewMode)
	{
	case 0:
	{
		WjCharacter->SetCharacterViewMode(ECharacterViewMode::TopDown);
		break;
	}
	case 1:
	{
		WjCharacter->SetCharacterViewMode(ECharacterViewMode::ThirdPerson);
		break;
	}
	case 2:
	{
		WjCharacter->SetCharacterViewMode(ECharacterViewMode::FirstPerson);
		break;
	}
	}
}
