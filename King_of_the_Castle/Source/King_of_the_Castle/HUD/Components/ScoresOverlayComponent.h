// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HUD/HUDComponent.h"
#include "ScoresOverlayComponent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API UScoresOverlayComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UScoresOverlayComponent();

	virtual void SetVisible(const bool& visible) override;

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	float m_BarAlpha;

	float m_BarMoveSpeed;

	FVector2D m_GraphSize;

private:
	TArray<float> m_Scores;
};
