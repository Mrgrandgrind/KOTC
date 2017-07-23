// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "Gamemode/BaseGameMode.h"
#include "Character/PlayerCharacter.h"

#include "Engine/Canvas.h"

#define CROSSHAIR_SIZE 0.016f // Multiplier of min(width, height)
#define CROSSHAIR_COLOR FLinearColor(0.25f, 0.75f, 0.75f, 0.5f)

#define HEALTH_COLOR FLinearColor(1.0f, 0.1f, 0.1f, 0.75f)
#define STAMINA_COLOR FLinearColor(0.1f, 0.8f, 0.1f, 0.75f)

AGameHUD::AGameHUD() : m_bCrosshairVisible(false)
{
	// Scale
	this->m_ScaleMaster = 1.0f;
	this->m_ScaleVertical = 0.003f;
	this->m_ScaleHorizontal = 0.005f;

	// Health and Stamina bars
	this->m_BarHealthColor = HEALTH_COLOR;
	this->m_BarStaminaColor = STAMINA_COLOR;
	this->m_BarHealthText = TEXT("HP");
	this->m_BarStaminaText = TEXT("SP");
	this->m_BarTextScale = 0.65f;
	this->m_BarScaleXHealth = 120.0f;
	this->m_BarScaleXStamina = 90.0f;
	this->m_BarScaleY = 10.0f;
	this->m_BarSeparation = 2.5f;
	this->m_BarPadding = 12.0f;
	this->m_bRenderBars = true;

	//this->m_BuildWheel = UObject::CreateDefaultSubobject<UBuildWheel>(TEXT("BuildWheel"));
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	this->m_PlayerCount = GetGameMode<ABaseGameMode>(Super::GetWorld())->GetPlayerCount();
	this->m_ControllerId = UGameplayStatics::GetPlayerControllerID(Super::GetOwningPlayerController());
}

void AGameHUD::RenderBars(const FVector4& screen, const float& scale)
{
	APlayerCharacter *character = this->GetCharacter();

	float padding = scale * this->m_BarPadding;
	float x = screen.X + padding, y = screen.Y + padding / 2.0f;

	float widthHealth = scale * this->m_BarScaleXHealth, widthStamina = scale * this->m_BarScaleXStamina;
	float height = scale * this->m_BarScaleY;

	// If the bars should be displayed on the opposite side (x)
	bool opposite = (this->m_PlayerCount == 2 && this->m_ControllerId == 1)
		|| (this->m_PlayerCount == 3 && this->m_ControllerId == 2)
		|| (this->m_PlayerCount == 4 && (this->m_ControllerId == 1 || this->m_ControllerId == 3));
	if (opposite)
	{
		x = screen.X + screen.Z - padding;
		widthHealth = -widthHealth;
		widthStamina = -widthStamina;
	}

	float textWidth, textHeight, offset;
	// Health
	float healthPerc = character->GetHealth() / character->GetMaxHealth();

	Super::GetTextSize(this->m_BarHealthText, textWidth, textHeight, nullptr, scale * this->m_BarTextScale);
	Super::DrawText(this->m_BarHealthText, FLinearColor::White, x - textWidth / 2.0f, 
		y + height / 2.0f - textHeight / 2.0f, nullptr, scale * this->m_BarTextScale);

	offset = opposite ? -(textWidth / 4.0f + padding / 2.0f) : (textWidth / 4.0f + padding / 2.0f);
	Super::DrawRect(this->m_BarHealthColor, x + offset, y, widthHealth * healthPerc, height * 0.6f);
	Super::DrawRect(this->m_BarHealthColor * 0.9f, x + offset, y + height * 0.6f, widthHealth * healthPerc, height * 0.4f);

	Super::DrawRect(this->m_BarHealthColor * 0.3f, x + offset + widthHealth * healthPerc, y, widthHealth * (1.0f - healthPerc), height);

	// Stamina
	float staminaPerc = character->GetStamina() / character->GetMaxStamina();

	y += height + scale * this->m_BarSeparation;

	Super::GetTextSize(this->m_BarHealthText, textWidth, textHeight, nullptr, scale * this->m_BarTextScale);
	Super::DrawText(this->m_BarStaminaText, FLinearColor::White, x - textWidth / 2.0f,
		y + height / 2.0f - textHeight / 2.0f, nullptr, scale * this->m_BarTextScale);

	offset = opposite ? -(textWidth / 4.0f + padding / 2.0f) : (textWidth / 4.0f + padding / 2.0f);
	Super::DrawRect(this->m_BarStaminaColor * 0.9f, x + offset, y, widthStamina * staminaPerc, height * 0.4f);
	Super::DrawRect(this->m_BarStaminaColor, x + offset, y + height * 0.4f, widthStamina * staminaPerc, height * 0.6f);

	Super::DrawRect(this->m_BarStaminaColor * 0.3f, x + offset + widthStamina * staminaPerc, y, widthStamina * (1.0f - staminaPerc), height);
}

void AGameHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Super::bShowHUD)
	{
		return;
	}
	FVector4 screen = FVector4(Super::Canvas->OrgX, Super::Canvas->OrgY, Super::Canvas->SizeX, Super::Canvas->SizeY);

	float scale = FMath::Min(screen.Z, screen.W);
	scale *= this->m_ScaleMaster;
	scale *= this->IsViewportVertical() ? this->m_ScaleVertical : this->m_ScaleHorizontal;

	// Render health and stamina bars
	if (this->m_bRenderBars)
	{
		this->RenderBars(screen, scale);
	}
	if (this->m_bCrosshairVisible)
	{
		const float& size = FMath::RoundToFloat(scale * CROSSHAIR_SIZE);
		Super::DrawRect(CROSSHAIR_COLOR, screen.X + screen.Z / 2.0f - size / 2.0f,
			screen.Y + screen.W / 2.0f - size / 2.0f, size, size);
	}
	//this->m_BuildWheel->Render(this, screen);
}
