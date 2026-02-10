// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/JumpMap/JumpMapCheckpointActor.h"
#include "Components/BoxComponent.h"
#include "Core/Play/WjWorldCharacterPlay.h"
#include "Core/Play/WjWorldPlayerStatePlay.h"
#include "Core/GameData/JumpMapPlayerDataComponent.h"
#include "WjWorldLogCategories.h"

AJumpMapCheckpointActor::AJumpMapCheckpointActor()
{
	CheckpointTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CheckpointTrigger"));
	CheckpointTrigger->SetupAttachment(RootComp);
	CheckpointTrigger->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	CheckpointTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CheckpointTrigger->SetGenerateOverlapEvents(true);
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
