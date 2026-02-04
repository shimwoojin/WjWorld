// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Cosmetic/WjWorldCosmeticComponent.h"
#include "Net/UnrealNetwork.h"
#include "WjWorldLogCategories.h"

AWjWorldPlayerStateBase::AWjWorldPlayerStateBase()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AWjWorldPlayerStateBase::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어인 경우 서버에 코스메틱 로드아웃 전송
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC && PC->IsLocalController())
	{
		UWjWorldCosmeticSubsystem* CS = GetGameInstance()->GetSubsystem<UWjWorldCosmeticSubsystem>();
		if (CS)
		{
			ServerSetCosmeticLoadout(CS->GetLoadout());
		}
	}
}

void AWjWorldPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWjWorldPlayerStateBase, bIsReady);
	DOREPLIFETIME(AWjWorldPlayerStateBase, CosmeticLoadout);
}

void AWjWorldPlayerStateBase::ToggleReady()
{
	// Client → Server RPC
	ServerSetReady(!bIsReady);
}

void AWjWorldPlayerStateBase::SetReady(bool bNewReady)
{
	// Server Only (직접 호출용)
	if (!HasAuthority())
	{
		UE_LOG(LogWjWorld, Warning, TEXT("PlayerState: SetReady called on non-authority"));
		return;
	}

	if (bIsReady != bNewReady)
	{
		bIsReady = bNewReady;
		OnRep_IsReady(); // 로컬에서도 이벤트 발생

		UE_LOG(LogWjWorld, Log, TEXT("PlayerState: %s ready state changed to %d"),
			*GetPlayerName(), bIsReady);
	}
}

void AWjWorldPlayerStateBase::SetCosmeticLoadout(const FCosmeticLoadout& InLoadout)
{
	if (!HasAuthority())
	{
		return;
	}

	CosmeticLoadout = InLoadout;

	// 서버에서도 즉시 적용
	OnRep_CosmeticLoadout();
}

void AWjWorldPlayerStateBase::OnCosmeticLoadoutUpdated()
{
	// 소유 Pawn의 CosmeticComponent에 로드아웃 적용
	if (APawn* OwnerPawn = GetPawn())
	{
		if (UWjWorldCosmeticComponent* CosmeticComp = OwnerPawn->FindComponentByClass<UWjWorldCosmeticComponent>())
		{
			CosmeticComp->ApplyLoadout(CosmeticLoadout);
			UE_LOG(LogWjWorldCosmetic, Log, TEXT("PlayerStateBase: 코스메틱 로드아웃 적용 (%s)"), *GetPlayerName());
		}
		bPendingCosmeticApply = false;
	}
	else
	{
		// Pawn이 아직 없으면 대기 플래그 설정
		bPendingCosmeticApply = true;
		UE_LOG(LogWjWorldCosmetic, Log, TEXT("PlayerStateBase: Pawn 없음, 코스메틱 적용 대기 (%s)"), *GetPlayerName());
	}
}

void AWjWorldPlayerStateBase::OnPawnSet(APawn* OldPawn, APawn* NewPawn)
{
	// 대기 중인 코스메틱 적용
	if (bPendingCosmeticApply && NewPawn)
	{
		if (UWjWorldCosmeticComponent* CosmeticComp = NewPawn->FindComponentByClass<UWjWorldCosmeticComponent>())
		{
			CosmeticComp->ApplyLoadout(CosmeticLoadout);
			bPendingCosmeticApply = false;
			UE_LOG(LogWjWorldCosmetic, Log, TEXT("PlayerStateBase: 대기 중인 코스메틱 적용 완료 (%s)"), *GetPlayerName());
		}
	}
}

void AWjWorldPlayerStateBase::OnRep_CosmeticLoadout()
{
	OnCosmeticLoadoutUpdated();
}

void AWjWorldPlayerStateBase::OnRep_IsReady()
{
	UE_LOG(LogWjWorld, Log, TEXT("PlayerState: %s ready state replicated - %d"),
		*GetPlayerName(), bIsReady);

	// 이벤트 브로드캐스트
	OnReadyStateChanged.Broadcast(GetPlayerId(), bIsReady);
}

void AWjWorldPlayerStateBase::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	UE_LOG(LogWjWorld, Warning, TEXT("PlayerState: OnRep_PlayerName - %s (ID: %d)"),
		*GetPlayerName(), GetPlayerId());

	// 이름이 업데이트되었음을 알림
	OnPlayerNameUpdated.Broadcast(GetPlayerName());
}

void AWjWorldPlayerStateBase::ServerSetReady_Implementation(bool bNewReady)
{
	SetReady(bNewReady);
}

void AWjWorldPlayerStateBase::ServerSetCosmeticLoadout_Implementation(const FCosmeticLoadout& InLoadout)
{
	SetCosmeticLoadout(InLoadout);
}
