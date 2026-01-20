// Fill out your copyright notice in the Description page of Project Settings.


#include "WjWorldGameplayTag.h"

WjWorldGameplayTag::WjWorldGameplayTag()
{
}

WjWorldGameplayTag::~WjWorldGameplayTag()
{
}

FGameplayTag WjWorldGameplayTag::Camera_ThirdPerson()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Camera.ThirdPerson"));
}

FGameplayTag WjWorldGameplayTag::Camera_FirstPerson()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Camera.FirstPerson"));
}

FGameplayTag WjWorldGameplayTag::Camera_TopDown()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Camera.TopDown"));
}
