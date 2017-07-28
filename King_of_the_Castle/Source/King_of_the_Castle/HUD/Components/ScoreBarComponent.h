// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HUD/HUDComponent.h"
#include "ScoreBarComponent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API UScoreBarComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UScoreBarComponent();
	
protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Time Text Scale"))
	float m_TimeTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Time Text Color"))
	FLinearColor m_TimeTextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Time Box Scale"))
	float m_TimeBoxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Time Box Color"))
	FLinearColor m_TimeBoxColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Render Time"))
	bool m_bRenderTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Width"))
	float m_BarWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Height"))
	float m_BarHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Alpha"))
	float m_BarAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Move Speed"))
	float m_BarMoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Text Scale"))
	float m_BarTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Text Color"))
	FLinearColor m_BarTextColor;
	
private:
	TArray<float> m_TeamBarScores;

};
