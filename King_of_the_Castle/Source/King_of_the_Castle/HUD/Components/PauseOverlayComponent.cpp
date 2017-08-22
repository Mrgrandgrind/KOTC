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

	float x = origin.X + 20.0f * scale, y = origin.Y + 20.0f * scale, width, height;

	FString text = TEXT("Paused");
	hud->GetTextSize(text, width, height, hud->GetFont(), 2.0f * scale);

	if (hud->IsOpposite())
	{
		x = origin.X + extent.X - width - 20.0f * scale;
	}
	hud->DrawText(text, FLinearColor(0.9f, 0.9f, 0.9f, 0.75f), x, y, hud->GetFont(), 2.0f * scale);
}