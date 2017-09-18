// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "CountdownComponent.h"

#define DEFAULT_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)

UCountdownComponent::UCountdownComponent()
{
	this->m_Counter = 0.0f;

	this->m_Duration = 5.0f;
	this->m_GoDuration = 1.0f;
	this->m_WaitDuration = 1.0f;
	this->m_TextScale = 4.0f;
	this->m_TextColor = DEFAULT_TEXT_COLOR;
	this->m_BkgScale = 1.18f;
	this->m_BkgMaterial = nullptr;

	Super::m_bRenderOnLast = true;
}

void UCountdownComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	this->m_Counter += hud->GetWorld()->GetDeltaSeconds();
	if (this->m_Counter >= this->m_Duration + this->m_GoDuration + this->m_WaitDuration)
	{
		Super::m_bRender = false;
		return;
	}

	bool bkg = true;
	FString text;
	float x = origin.X, y = origin.Y, width, height;
	if (this->m_Counter <= this->m_Duration + this->m_WaitDuration - this->m_GoDuration + 1.0f)
	{
		text = FString::Printf(TEXT("%d"), FMath::Min(int(this->m_Duration),
			int(this->m_Duration - (this->m_Counter - this->m_WaitDuration)) + 1));
	}
	else
	{
		bkg = false;
		text = TEXT("GO!");
	}
	hud->GetTextSize(text, width, height, hud->GetFont(), this->m_TextScale * scale);
	if(bkg && this->m_BkgMaterial != nullptr)
	{
		float size = FMath::Max(width, height) * this->m_BkgScale;
		hud->DrawMaterialSimple(this->m_BkgMaterial, x - size / 2.0f, y - size / 2.0f - 6, size, size);
	}
	hud->DrawText(text, this->m_TextColor, x - width / 2.0f, y - height / 2.0f, hud->GetFont(), this->m_TextScale * scale);
}