// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "StatBarsComponent.h"

#include "Character/PlayerCharacter.h"

#define HEALTH_FULL_COLOR FLinearColor(0.1f, 0.8f, 0.1f, 0.75f)
#define HEALTH_EMPTY_COLOR FLinearColor(1.0f, 0.1f, 0.1f, 0.75f)
#define STAMINA_COLOR FLinearColor(0.1f, 0.2f, 0.9f, 0.75f)

UStatBarsComponent::UStatBarsComponent()
{
	this->m_HealthMaxColor = HEALTH_FULL_COLOR;
	this->m_HealthMinColor = HEALTH_EMPTY_COLOR;
	this->m_StaminaColor = STAMINA_COLOR;
	this->m_HealthText = TEXT("HP");
	this->m_StaminaText = TEXT("SP");
	this->m_TextScale = 0.4f;
	this->m_ScaleXHealth = 120.0f;
	this->m_ScaleXStamina = 90.0f;
	this->m_ScaleY = 10.0f;
	this->m_SeparationY = 2.5f;
	this->m_MoveSpeed = 2.0f;

	Super::m_Padding = FVector2D(4.0f, 4.0f);
}

void UStatBarsComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	APlayerCharacter *character = hud->GetCharacter();
	float x = origin.X, y = origin.Y;

	bool opposite = hud->IsOpposite();
	float widthHealth = this->m_ScaleXHealth * scale, widthStamina = this->m_ScaleXStamina * scale;
	float height = this->m_ScaleY * scale;

	// If the bars should be displayed on the opposite side (x)
	if (opposite)
	{
		x = origin.X + extent.X;
		widthHealth = -widthHealth;
		widthStamina = -widthStamina;
	}

	float textWidth, textHeight, offset;
	// Health
	float healthPerc = character->GetHealth() / character->GetMaxHealth();
	healthPerc = this->m_HealthPerc = FMath::FInterpTo(this->m_HealthPerc, healthPerc,
		hud->GetWorld()->GetDeltaSeconds(), this->m_MoveSpeed);

	FLinearColor healthColor = this->m_HealthMinColor + (m_HealthMaxColor - m_HealthMinColor) * healthPerc;

	hud->GetTextSize(this->m_HealthText, textWidth, textHeight, hud->GetFont(), this->m_TextScale * scale);
	hud->DrawText(this->m_HealthText, FLinearColor::White, opposite ? x - textWidth : x, y + height / 2.0f - textHeight / 2.0f, hud->GetFont(), this->m_TextScale * scale);

	offset = (opposite ? -textWidth : textWidth) * 1.25f;
	hud->DrawRect(healthColor, x + offset, y, widthHealth * healthPerc, height * 0.6f);
	hud->DrawRect(healthColor * 0.8f, x + offset, y + height * 0.6f, widthHealth * healthPerc, height * 0.4f);

	hud->DrawRect(healthColor * 0.4f, x + offset + widthHealth * healthPerc, y, widthHealth * (1.0f - healthPerc), height);

	// Stamina
	float staminaPerc = character->GetStamina() / character->GetMaxStamina();
	staminaPerc = this->m_StaminaPerc = FMath::FInterpTo(this->m_StaminaPerc, staminaPerc,
		hud->GetWorld()->GetDeltaSeconds(), this->m_MoveSpeed);

	y += height + this->m_SeparationY * scale;

	hud->GetTextSize(this->m_HealthText, textWidth, textHeight, hud->GetFont(), scale * this->m_TextScale);
	hud->DrawText(this->m_StaminaText, FLinearColor::White, opposite ? x - textWidth : x, y + height / 2.0f - textHeight / 2.0f, hud->GetFont(), this->m_TextScale * scale);

	offset = (opposite ? -textWidth : textWidth) * 1.25f;
	hud->DrawRect(this->m_StaminaColor * 0.8f, x + offset, y, widthStamina * staminaPerc, height * 0.4f);
	hud->DrawRect(this->m_StaminaColor, x + offset, y + height * 0.4f, widthStamina * staminaPerc, height * 0.6f);

	hud->DrawRect(this->m_StaminaColor * 0.4f, x + offset + widthStamina * staminaPerc, y, widthStamina * (1.0f - staminaPerc), height);
}