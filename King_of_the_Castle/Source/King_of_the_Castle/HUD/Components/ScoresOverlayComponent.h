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

	FORCEINLINE void SetTitleText(FString text) { this->m_TitleText = text; }

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Title Text"))
	FString m_TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Title Box Scale"))
	float m_TitleBoxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Title Text Scale"))
	float m_TitleTextScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Title Text Color"))
	FLinearColor m_TitleTextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Background Color"))
	FLinearColor m_BkgColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Alpha"))
	float m_BarAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Move Speed"))
	float m_BarMoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Score Text Scale"))
	float m_ScoreTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Goal Height Offset"))
	float m_GoalHeightOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Goal Line Height"))
	float m_GoalLineHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Goal Text Scale"))
	float m_GoalTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Goal Color"))
	FLinearColor m_GoalColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Graph Y Increments"))
	float m_GraphYIncrements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Graph Size"))
	FVector2D m_GraphSize;

private:
	TArray<float> m_Scores;
};
