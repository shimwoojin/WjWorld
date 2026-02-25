// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Base/WjWorldPlayerControllerBase.h"
#include "WjWorldPlayerControllerPlay.generated.h"

class UConfirmDialogWidget;

/**
 * Play 모드 전용 플레이어 컨트롤러
 * - ESC 키로 퇴장 다이얼로그 표시
 */
UCLASS()
class WJWORLD_API AWjWorldPlayerControllerPlay : public AWjWorldPlayerControllerBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	/** ESC 키 핸들러: LeaveDialog 토글 (Base의 팝업 닫기보다 우선) */
	virtual void OnEscapePressed() override;

private:

	/** 퇴장 확인 다이얼로그 표시 */
	void ShowLeaveGameDialog();

	/** 퇴장 확인 다이얼로그 닫기 */
	void CloseLeaveGameDialog();

	/** 확인 콜백 */
	UFUNCTION()
	void OnLeaveGameConfirmed();

	/** 취소 콜백 */
	UFUNCTION()
	void OnLeaveGameCancelled();

	/** 실제 퇴장 처리 (LeaveRoom + OpenLevel 로비) */
	void ExecuteLeaveGame();

private:
	/** 퇴장 다이얼로그 위젯 클래스 (BP에서 WBP_ConfirmDialog 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UConfirmDialogWidget> LeaveGameDialogClass;

	/** 퇴장 다이얼로그 인스턴스 */
	UPROPERTY()
	TObjectPtr<UConfirmDialogWidget> LeaveGameDialog;

	/** 다이얼로그 열림 상태 */
	bool bIsLeaveDialogOpen = false;
};
