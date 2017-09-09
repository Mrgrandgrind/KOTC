// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HUD/HUDComponent.h"
#include "CapturePointsComponent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API UCapturePointsComponent : public UHUDComponent
{
	GENERATED_BODY()

public:
	UCapturePointsComponent();
	
protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Box Owned Color"))
	FLinearColor m_OwnedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Box Unowned Color"))
	FLinearColor m_UnownedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "On Screen Radius Scale"))
	float m_PositionRadiusScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Always On Screen"))
	float m_bAlwaysOnScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Box Scale"))
	float m_BoxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Team Box UV Size"))
	float m_TeamBoxUVSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Box Alpha"))
	float m_BoxAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Box Flash Speed"))
	float m_BoxFlashSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Arrow Angle (deg)"))
	float m_ArrowAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Arrow Thickness"))
	float m_ArrowTickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Arrow Dist Offset (0)"))
	float m_ArrowDistOffset0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Arrow Dist Offset (1/2)"))
	float m_ArrowDistOffset12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Arrow Color"))
	FLinearColor m_ArrowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Render Arrow"))
	bool m_bRenderArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Color"))
	FLinearColor m_TextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Scale"))
	float m_TextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Y Offset"))
	float m_TextZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Shadow Offset"))
	FVector2D m_TextShadowOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Capture Point Material"))
	UMaterialInterface *m_Material;

private:
	// Each flag will have their own material instance because DrawMaterial doesn't immediately draw and material parameters get overwritten
	UPROPERTY()
	TMap<class ACapturePoint*, UMaterialInterface*> m_Materials;
};
