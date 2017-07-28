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
	// Only for horizontal
	void DrawDashedRect(class AGameHUD *hud, const FLinearColor& color, const float& x, const float& y, const float& width, const float& height, const float& gap);

	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	float m_TitleBoxScale;

	float m_TitleTextScale;

	FLinearColor m_TitleTextColor;

	float m_BarAlpha;

	float m_BarMoveSpeed;

	float m_GoalHeightOffset;

	float m_GoalLineHeight;

	float m_GoalTextScale;

	FLinearColor m_GoalColor;

	float m_GraphYIncrements;

	FVector2D m_GraphSize;

private:
	TArray<float> m_Scores;
};
