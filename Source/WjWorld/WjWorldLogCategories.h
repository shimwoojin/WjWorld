// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * WjWorld 프로젝트 전용 로그 카테고리
 * 
 * 사용법:
 * UE_LOG(LogWjWorld, Log, TEXT("메시지"));
 * UE_LOG(LogWjWorld, Warning, TEXT("경고"));
 * UE_LOG(LogWjWorld, Error, TEXT("에러"));
 */

// 로그 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogWjWorld, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWjWorldAbilities, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWjWorldCosmetic, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWjWorldStats, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWjWorldPlacement, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWjWorldCurrency, Log, All);
