// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WjWorldBrickPreviewActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class WJWORLD_API AWjWorldBrickPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AWjWorldBrickPreviewActor();

	virtual void BeginPlay() override;

	void InitializePreview(const FVector& InSize);
	void UpdatePreviewLocation(const FVector& InLocation);
	void SetPreviewValid(bool bIsValid);
	void SetBrickTypeColor(const FLinearColor& InColor);

	bool IsPreviewValid() const { return bIsValid; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// 에디터에서 색상 조정 가능
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);  // 반투명 녹색

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);  // 반투명 빨강

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	float PreviewOpacity = 0.5f;

private:
	bool bIsValid = true;
	bool bHasBrickTypeColor = false;
	FLinearColor BrickTypeColor;
};
