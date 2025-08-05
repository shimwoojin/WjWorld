// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Interact/InteractablePortal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "UI/Interact/InteractionWidget.h"

AInteractablePortal::AInteractablePortal()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root Component 설정
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // Portal Mesh 설정
    PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
    PortalMesh->SetupAttachment(RootComponent);

    // Interaction Trigger 설정
    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Widget Component 설정
    InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
    InteractionWidgetComponent->SetupAttachment(RootComponent);
    InteractionWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    InteractionWidgetComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
    InteractionWidgetComponent->SetVisibility(false);

    // 기본값 설정
    PortalDisplayName = TEXT("Portal");
    TargetLevelName = TEXT("DefaultLevel");
    PortalType = EPortalType::Custom;
}

void AInteractablePortal::BeginPlay()
{
    Super::BeginPlay();

    // 이벤트 바인딩
    if (InteractionTrigger)
    {
        InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AInteractablePortal::OnTriggerBeginOverlap);
        InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AInteractablePortal::OnTriggerEndOverlap);
    }

    // UI 위젯 설정
    if (InteractionWidgetClass && InteractionWidgetComponent)
    {
        InteractionWidget = CreateWidget<UInteractionWidget>(GetWorld(), InteractionWidgetClass);
        if (InteractionWidget)
        {
            InteractionWidgetComponent->SetWidget(InteractionWidget);
            UpdateInteractionUI();
        }
    }
}

//void AInteractablePortal::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//    Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//    // F키 바인딩 (기본 입력)
//    if (PlayerInputComponent)
//    {
//        PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AInteractablePortal::OnInteract);
//    }
//}

void AInteractablePortal::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
        if (Pawn->IsPlayerControlled())
        {
            bPlayerInRange = true;
            InteractingPlayer = Pawn;
            bCanInteract = true;

            ShowInteractionUI();

            UE_LOG(LogTemp, Warning, TEXT("Player entered portal range: %s"), *PortalDisplayName);
        }
    }
}

void AInteractablePortal::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
        if (Pawn->IsPlayerControlled())
        {
            bPlayerInRange = false;
            InteractingPlayer = nullptr;
            bCanInteract = false;

            HideInteractionUI();

            UE_LOG(LogTemp, Warning, TEXT("Player left portal range: %s"), *PortalDisplayName);
        }
    }
}

void AInteractablePortal::OnInteract()
{
    if (bCanInteract && bPlayerInRange && InteractingPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Portal interaction triggered: %s"), *PortalDisplayName);

        // 블루프린트 이벤트 호출
        OnPortalActivated();

        // 레벨 이동
        TravelToTargetLevel();
    }
}

void AInteractablePortal::ShowInteractionUI()
{
    if (InteractionWidgetComponent)
    {
        InteractionWidgetComponent->SetVisibility(true);
        UpdateInteractionUI();
    }
}

void AInteractablePortal::HideInteractionUI()
{
    if (InteractionWidgetComponent)
    {
        InteractionWidgetComponent->SetVisibility(false);
    }
}

void AInteractablePortal::UpdateInteractionUI()
{
    if (InteractionWidget)
    {
        FString InteractionText = FString::Printf(TEXT("Press %s to enter %s"),
            *InteractionKey, *PortalDisplayName);
        InteractionWidget->SetInteractionText(InteractionText);
    }
}

void AInteractablePortal::TravelToTargetLevel()
{
    if (!TargetLevelName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Traveling to level: %s"), *TargetLevelName);
        UGameplayStatics::OpenLevel(GetWorld(), FName(*TargetLevelName));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TargetLevelName is empty for portal: %s"), *PortalDisplayName);
    }
}

void AInteractablePortal::SetPortalEnabled(bool bEnabled)
{
    bCanInteract = bEnabled;
    InteractionTrigger->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

    if (!bEnabled)
    {
        HideInteractionUI();
    }
}

