// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Base/WjWorldPlayerStateBase.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
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
	// 베이스 클래스에서는 아무것도 하지 않음. 서브클래스에서 오버라이드.
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
