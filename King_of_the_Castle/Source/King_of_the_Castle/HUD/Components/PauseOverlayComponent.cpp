// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "PauseOverlayComponent.h"

#include "Gamemode/BaseGameMode.h"
#include "Character/PlayerCharacter.h"

UPauseOverlayComponent::UPauseOverlayComponent()
{
	Super::m_bRenderWhenPaused = true;
}

void UPauseOverlayComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	if (!hud->IsPaused())
	{
		return;
	}
	FLinearColor color = GetGameMode(hud->GetWorld())->GetTeamColor(hud->GetCharacter()->GetTeam());
	color *= 0.005f;
	color.A = 0.8f;

	hud->DrawRect(color, origin.X, origin.Y, extent.X, extent.Y);
}