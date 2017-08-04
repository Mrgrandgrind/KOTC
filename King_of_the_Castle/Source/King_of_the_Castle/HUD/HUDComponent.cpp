// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "HUDComponent.h"

UHUDComponent::UHUDComponent()
{
	this->m_MasterScale = 1.0f;
	this->m_OriginOffset = FVector2D(0.0f, 0.0f);
	this->m_bRender = true;
	this->m_bRenderWhenPaused = false;
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