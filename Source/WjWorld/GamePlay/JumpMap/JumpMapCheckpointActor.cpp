// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapCheckpointActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/JumpMapPlayerDataComponent.h"
#include "WjWorldLogCategories.h"

AJumpMapCheckpointActor::AJumpMapCheckpointActor()
{
	JumpMapObjectId = TEXT("Checkpoint");
	CheckpointTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CheckpointTrigger"));
	CheckpointTrigger->SetupAttachment(RootComp);
	CheckpointTrigger->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	CheckpointTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CheckpointTrigger->SetGenerateOverlapEvents(true);
}

void AJumpMapCheckpointActor::GetSerializableProperties(TMap<FString, FString>& OutProperties) const
{
	OutProperties.Add(TEXT("CheckpointOrder"), FString::FromInt(CheckpointOrder));
	OutProperties.Add(TEXT("RespawnOffset"), FString::Printf(TEXT("%.2f;%.2f;%.2f"), RespawnOffset.X, RespawnOffset.Y, RespawnOffset.Z));
	const FVector Extent = CheckpointTrigger->GetUnscaledBoxExtent();
	OutProperties.Add(TEXT("BoxExtent"), FString::Printf(TEXT("%.2f;%.2f;%.2f"), Extent.X, Extent.Y, Extent.Z));
}

void AJumpMapCheckpointActor::ApplySerializedProperties(const TMap<FString, FString>& Properties)
{
	if (const FString* Value = Properties.Find(TEXT("CheckpointOrder")))
	{
		CheckpointOrder = FCString::Atoi(**Value);
	}
	if (const FString* Value = Properties.Find(TEXT("RespawnOffset")))
	{
		TArray<FString> Components;
		Value->ParseIntoArray(Components, TEXT(";"), true);
		if (Components.Num() >= 3)
		{
			RespawnOffset = FVector(FCString::Atof(*Components[0]), FCString::Atof(*Components[1]), FCString::Atof(*Components[2]));
		}
	}
	if (const FString* Value = Properties.Find(TEXT("BoxExtent")))
	{
		TArray<FString> Components;
		Value->ParseIntoArray(Components, TEXT(";"), true);
		if (Components.Num() >= 3)
		{
			CheckpointTrigger->SetBoxExtent(FVector(FCString::Atof(*Components[0]), FCString::Atof(*Components[1]), FCString::Atof(*Components[2])));
		}
	}
}

void AJumpMapCheckpointActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CheckpointTrigger->OnComponentBeginOverlap.AddDynamic(this, &AJumpMapCheckpointActor::OnCheckpointOverlap);
	}
}

void AJumpMapCheckpointActor::OnCheckpointOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AWjWorldCharacterPlay* Character = Cast<AWjWorldCharacterPlay>(OtherActor);
	if (!Character || Character->IsEliminated()) return;

	// PlayerData에서 체크포인트 인덱스 갱신 (진행 방향만 허용)
	AWjWorldPlayerStatePlay* PS = Character->GetPlayerState<AWjWorldPlayerStatePlay>();
	if (!PS) return;

	UJumpMapPlayerDataComponent* PlayerData = PS->GetGameData<UJumpMapPlayerDataComponent>();
	if (!PlayerData) return;

	// 현재보다 높은 순서의 체크포인트만 갱신 (역주행 방지)
	if (CheckpointOrder > PlayerData->GetCurrentCheckpointIndex())
	{
		PlayerData->SetCurrentCheckpointIndex(CheckpointOrder);
		UE_LOG(LogWjWorld, Log, TEXT("JumpMapCheckpoint[%d]: Player [%s] reached checkpoint"),
			CheckpointOrder, *Character->GetName());
	}
}
