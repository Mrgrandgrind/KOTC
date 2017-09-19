// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HUD/HUDComponent.h"
#include "GameOverCounterComponent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API UGameOverCounterComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UGameOverCounterComponent();

	inline void Reset() { this->m_Counter = 0.0f; }

	inline bool IsReady() const { return this->m_Counter >= this->m_Duration; }

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	float m_Size;

	float m_Duration;

	float m_FadeInDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Count Material"))
	UMaterialInterface *m_CountMaterial;

private:
	float m_Counter;

	UPROPERTY()
	UMaterialInterface *m_CountMaterialDynamic;
};
