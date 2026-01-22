#pragma once
#include "CoreMinimal.h"
#include "WjWorldCoreTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	None	    UMETA(DisplayName = "None"),
	Ready       UMETA(DisplayName = "Ready"),
	Playing     UMETA(DisplayName = "Playing"),
	Finished    UMETA(DisplayName = "Finished")
};