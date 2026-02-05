// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/WjWorldLanNetDriver.h"
#include "SocketSubsystem.h"

ISocketSubsystem* UWjWorldLanNetDriver::GetSocketSubsystem()
{
	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
}
