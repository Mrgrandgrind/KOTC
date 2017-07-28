// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "CrosshairComponent.h"

#define CROSSHAIR_COLOR FLinearColor(0.25f, 0.75f, 0.75f, 0.5f)

UCrosshairComponent::UCrosshairComponent()
{
	this->m_CrosshairSize = 5.0f;
	this->m_CrosshairColor = CROSSHAIR_COLOR;

	Super::m_bRender = false;
}

void UCrosshairComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	const float& size = FMath::RoundToFloat(this->m_CrosshairSize * scale);
	hud->DrawRect(this->m_CrosshairColor, origin.X + extent.X / 2.0f - size / 2.0f,
		origin.Y + extent.Y / 2.0f - size / 2.0f, size, size);
}