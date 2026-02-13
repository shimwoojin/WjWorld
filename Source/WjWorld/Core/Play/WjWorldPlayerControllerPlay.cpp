// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Play/WjWorldPlayerControllerPlay.h"

void AWjWorldPlayerControllerPlay::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
}

