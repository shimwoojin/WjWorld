// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/TriggerBox.h"
#include "InteractablePortal.generated.h"

class UInteractionWidget;

UENUM(BlueprintType)
enum class EPortalType : uint8
{
    MiniGame    UMETA(DisplayName = "Mini Game"),
    Multiplayer UMETA(DisplayName = "Multiplayer"),
    Tutorial    UMETA(DisplayName = "Tutorial"),
    Custom      UMETA(DisplayName = "Custom")
};

UCLASS()
class WJWORLD_API AInteractablePortal : public AActor
{
    GENERATED_BODY()

public:
    AInteractablePortal();

protected:
    virtual void BeginPlay() override;
    //virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal")
    void OnPortalActivated();

    // === Components ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PortalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* InteractionWidgetComponent;

    // === Portal Settings ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
    EPortalType PortalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
    FString TargetLevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
    FString PortalDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal Settings")
    FString InteractionKey = TEXT("F");

    // === UI Settings ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UInteractionWidget> InteractionWidgetClass;

    // === Input Settings ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* InteractAction;

private:
    // === State Variables ===
    UPROPERTY()
    UInteractionWidget* InteractionWidget;

    UPROPERTY()
    APawn* InteractingPlayer;

    bool bCanInteract = false;
    bool bPlayerInRange = false;

    // === Event Functions ===
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // === Interaction Functions ===
    UFUNCTION()
    void OnInteract();

    void ShowInteractionUI();
    void HideInteractionUI();
    void UpdateInteractionUI();

    // === Portal Functions ===

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void TravelToTargetLevel();

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void SetPortalEnabled(bool bEnabled);

public:
    // === Public Interface ===
    UFUNCTION(BlueprintCallable, Category = "Portal")
    bool IsPlayerInRange() const { return bPlayerInRange; }

    UFUNCTION(BlueprintCallable, Category = "Portal")
    FString GetPortalDisplayName() const { return PortalDisplayName; }

    UFUNCTION(BlueprintCallable, Category = "Portal")
    EPortalType GetPortalType() const { return PortalType; }
};