// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "WjWorldLanNetDriver.generated.h"

/**
 * LAN 전용 NetDriver
 *
 * SocketSubsystemSteamIP 플러그인이 기본 소켓 서브시스템을 Steam으로 오버라이드하면
 * IpNetDriver가 SteamSocketsP2P 주소로 바인딩을 시도하여 실패함.
 * 이 클래스는 플랫폼 기본 소켓 서브시스템(IPv4)을 명시적으로 사용하여 문제 해결.
 */
UCLASS(transient, config=Engine)
class WJWORLD_API UWjWorldLanNetDriver : public UIpNetDriver
{
	GENERATED_BODY()

public:
	/** 플랫폼 기본 소켓 서브시스템 사용 (Steam 소켓 오버라이드 무시) */
	virtual ISocketSubsystem* GetSocketSubsystem() override;
};
