// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifeHUD.h"

void AStrifeHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter = ViewportSize*0.5f;

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		
		if(HUDPackage.CrosshairCenter)
		{
			FVector2D Spread =  FVector2D::Zero();
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread);
		}
		if(HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread);
		}
	}
}

void AStrifeHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint = ViewportCenter - FVector2D(TextureWidth/2, TextureHeight/2) + Spread;
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f);
}