// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "CrosshairComponent.h"

#include "Gamemode/GMCapturePoints.h"
#include "Character/PlayerCharacter.h"

//#define CROSSHAIR_COLOR FLinearColor(0.25f, 0.75f, 0.75f, 0.5f)

UCrosshairComponent::UCrosshairComponent()
{
	this->m_CrosshairSize = 3.6f;
	this->m_CrosshairAlpha = 0.5f;

	Super::m_bRender = false;
}

void UCrosshairComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(hud->GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	FLinearColor color = gamemode->GetTeamColor(hud->GetCharacter()->GetTeam());
	color.A = this->m_CrosshairAlpha;

	const float& size = FMath::RoundToFloat(this->m_CrosshairSize * scale);
	hud->DrawRect(color, origin.X + extent.X / 2.0f - size / 2.0f,
		origin.Y + extent.Y / 2.0f - size / 2.0f, size, size);
}