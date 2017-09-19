// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "HUDComponent.h"

UHUDComponent::UHUDComponent()
{
	this->m_MasterScale = 1.0f;
	this->m_OriginOffset = FVector2D(0.0f, 0.0f);
	this->m_bRender = true;
	this->m_bRenderOnLast = false;
	this->m_bRenderWhenPaused = false;
}

void UHUDComponent::DrawDashedRect(class AGameHUD *hud, const FLinearColor& color,
	const float& x, const float& y, const float& width, const float& height, const float& gap) const
{
	for (float x2 = x; x2 < x + width; x2 += gap * 2.0f)
	{
		hud->DrawRect(color, x2, y - height / 2.0f, gap, height);
	}
}

void UHUDComponent::DrawBorderBox(AGameHUD* hud, const FLinearColor& foreground, const float& x, const float& y,
	const float& width, const float& height, const float& borderSize, const FLinearColor& border)
{
	hud->DrawRect(foreground, x, y, width, height);

	//left border
	hud->DrawRect(border, x - borderSize, y, borderSize, height);

	//right border
	hud->DrawRect(border, x + width, y, borderSize, height);

	//top border
	hud->DrawRect(border, x - borderSize, y - borderSize, width + borderSize * 2.0f, borderSize);

	//bottom border
	hud->DrawRect(border, x - borderSize, y + height, width + borderSize * 2.0f, borderSize);
}

void UHUDComponent::DrawComponent(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale, const bool& forceRender)
{
	if (!this->m_bRender && !forceRender)
	{
		return;
	}
	if (this->m_bRenderOnLast && hud->GetControllerId() != hud->GetPlayerCount() - 1)
	{
		this->m_bRender = false;
		return;
	}
	FVector2D padding = this->m_Padding * scale;
	FVector2D eOrigin = origin + this->m_OriginOffset + padding, eExtent = extent - padding * 2.0f;
	this->Render(hud, eOrigin, eExtent, scale * this->m_MasterScale);
}
