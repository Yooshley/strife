// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StrifeHUD.generated.h"

USTRUCT()
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairCenter;
	class UTexture2D* CrosshairLeft;
	class UTexture2D* CrosshairRight;
	class UTexture2D* CrosshairTop;
	class UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 * 
 */
UCLASS()
class STRIFE_API AStrifeHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	
	UPROPERTY(EditAnywhere, category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
