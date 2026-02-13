// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "WjWorldPlayerControllerPlay.generated.h"

/**
 * 
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerControllerPlay : public AWjWorldPlayerControllerBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
