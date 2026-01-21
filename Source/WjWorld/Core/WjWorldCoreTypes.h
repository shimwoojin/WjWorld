#pragma once
#include "CoreMinimal.h"
#include "WjWorldCoreTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Ready       UMETA(DisplayName = "Ready"),
	Playing     UMETA(DisplayName = "Playing"),
	Finished    UMETA(DisplayName = "Finished")
};