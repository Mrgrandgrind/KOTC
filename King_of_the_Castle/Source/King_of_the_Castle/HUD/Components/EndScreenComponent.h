// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HUD/HUDComponent.h"
#include "EndScreenComponent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API UEndScreenComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UEndScreenComponent();

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	float m_Counter;

	float m_WaitTimer;

	float m_ContainerWidth;

	float m_ContainerHeight;

	float m_ContainerWaitDelay;

	float m_ContainerFadeTimer;

	float m_ContainerBorderSize;

	FLinearColor m_ContainerBkgColor;

	FLinearColor m_ContainerBorderColor;

	float m_TitleTextY;

	float m_TitleTextSize;

	FString m_TitleText;

	FLinearColor m_TitleTextColor;

	float m_PlaceSize;

	float m_BkgFadeTimer;

	FLinearColor m_BkgColor;
};
