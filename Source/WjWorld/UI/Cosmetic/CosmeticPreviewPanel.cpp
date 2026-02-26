// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Cosmetic/CosmeticPreviewPanel.h"
#include "UI/Profile/CharacterPreviewActor.h"
#include "Cosmetic/WjWorldCosmeticSubsystem.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "WjWorldLogCategories.h"

void UCosmeticPreviewPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (RotateLeftButton)
	{
		RotateLeftButton->OnClicked.AddDynamic(this, &UCosmeticPreviewPanel::OnRotateLeftClicked);
	}

	if (RotateRightButton)
	{
		RotateRightButton->OnClicked.AddDynamic(this, &UCosmeticPreviewPanel::OnRotateRightClicked);
	}

	// 프리뷰 액터 생성 및 현재 로드아웃 표시
	CreatePreviewActor();
	ShowCurrentLoadout();
}

void UCosmeticPreviewPanel::NativeDestruct()
{
	DestroyPreviewActor();
	Super::NativeDestruct();
}

void UCosmeticPreviewPanel::ShowCurrentLoadout()
{
	bIsTryingOn = false;

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UWjWorldCosmeticSubsystem* CosmeticSub = GI->GetSubsystem<UWjWorldCosmeticSubsystem>();
	if (!CosmeticSub)
	{
		return;
	}

	OriginalLoadout = CosmeticSub->GetLoadout();
	PreviewLoadout = OriginalLoadout;

	if (PreviewActor)
	{
		PreviewActor->SetupPreview(OriginalLoadout);
		ApplyRenderTargetToImage();
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticPreviewPanel: Showing current loadout"));
}

void UCosmeticPreviewPanel::PreviewItem(ECosmeticSlot CosmeticSlot, FName ItemId)
{
	if (CosmeticSlot == ECosmeticSlot::None)
	{
		return;
	}

	bIsTryingOn = true;

	// 시착 로드아웃 업데이트
	PreviewLoadout.Equip(CosmeticSlot, ItemId);

	if (PreviewActor)
	{
		PreviewActor->SetupPreview(PreviewLoadout);
		ApplyRenderTargetToImage();
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticPreviewPanel: Previewing item %s on slot %d"), *ItemId.ToString(), static_cast<int32>(CosmeticSlot));
}

void UCosmeticPreviewPanel::ResetPreview()
{
	if (!bIsTryingOn)
	{
		return;
	}

	bIsTryingOn = false;
	PreviewLoadout = OriginalLoadout;

	if (PreviewActor)
	{
		PreviewActor->SetupPreview(OriginalLoadout);
		ApplyRenderTargetToImage();
	}

	UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticPreviewPanel: Preview reset to original loadout"));
}

void UCosmeticPreviewPanel::RotatePreview(float DeltaYaw)
{
	if (PreviewActor)
	{
		FRotator CurrentRotation = PreviewActor->GetActorRotation();
		CurrentRotation.Yaw += DeltaYaw;
		PreviewActor->SetActorRotation(CurrentRotation);
		PreviewActor->RefreshCapture();
		ApplyRenderTargetToImage();
	}
}

void UCosmeticPreviewPanel::OnRotateLeftClicked()
{
	RotatePreview(-RotateAngle);
}

void UCosmeticPreviewPanel::OnRotateRightClicked()
{
	RotatePreview(RotateAngle);
}

void UCosmeticPreviewPanel::CreatePreviewActor()
{
	if (PreviewActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 프리뷰 액터 클래스가 설정되지 않은 경우 기본 클래스 사용
	TSubclassOf<ACharacterPreviewActor> ActorClass = PreviewActorClass;
	if (!ActorClass)
	{
		ActorClass = ACharacterPreviewActor::StaticClass();
	}

	// 월드 밖 위치에 스폰 (화면에 보이지 않음)
	FVector SpawnLocation(0.f, 0.f, 15000.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewActor = World->SpawnActor<ACharacterPreviewActor>(ActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (PreviewActor)
	{
		// 로컬 플레이어의 Pawn에서 SkeletalMesh와 AnimBlueprint 복사
		APlayerController* PC = GetOwningPlayer();
		if (PC && PC->GetPawn())
		{
			PreviewActor->SetupFromPawn(PC->GetPawn());
		}

		UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticPreviewPanel: PreviewActor created"));
	}
}

void UCosmeticPreviewPanel::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;

		UE_LOG(LogWjWorldCosmetic, Log, TEXT("CosmeticPreviewPanel: PreviewActor destroyed"));
	}
}

void UCosmeticPreviewPanel::ApplyRenderTargetToImage()
{
	if (!PreviewImage || !PreviewActor)
	{
		return;
	}

	UMaterialInstanceDynamic* MID = PreviewActor->GetPreviewMaterial();
	if (MID)
	{
		PreviewImage->SetBrushResourceObject(MID);
	}
	else
	{
		// 머티리얼 폴백: RenderTarget 직접 사용
		UTextureRenderTarget2D* RenderTarget = PreviewActor->GetRenderTarget();
		if (RenderTarget)
		{
			PreviewImage->SetBrushResourceObject(RenderTarget);
		}
	}
}
