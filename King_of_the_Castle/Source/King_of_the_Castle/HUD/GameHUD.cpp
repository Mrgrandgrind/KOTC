// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "HUD/HUDComponent.h"
#include "Character/PlayerCharacter.h"
#include "Gamemode/BaseGameMode.h"
#include "Gamemode/GMCapturePoints.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#include "Engine/Canvas.h"

#define FONT_LOCATION TEXT("Font'/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField'")

AGameHUD::AGameHUD() 
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(FONT_LOCATION);
	if (Font.Succeeded())
	{
		this->m_Font = Font.Object;
	}
	this->m_ScaleMaster = 1.0f;
	this->m_ScaleVertical = 0.003f;
	this->m_ScaleHorizontal = 0.005f;
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	this->m_PlayerCount = GetGameMode<ABaseGameMode>(Super::GetWorld())->GetPlayerCount();
	this->m_ControllerId = UGameplayStatics::GetPlayerControllerID(Super::GetOwningPlayerController());

	for (TSubclassOf<UHUDComponent> next : this->m_ComponentClasses)
	{
		if (next == nullptr)
		{
			continue;
		}
		this->m_Components.Add(NewObject<UHUDComponent>(this, next));
	}
}

void AGameHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Super::bShowHUD || this->GetCharacter() == nullptr)
	{
		return;
	}
	FVector2D origin(Super::Canvas->OrgX, Super::Canvas->OrgY), extent(Super::Canvas->SizeX, Super::Canvas->SizeY);
	FVector4 screen = FVector4(Super::Canvas->OrgX, Super::Canvas->OrgY, Super::Canvas->SizeX, Super::Canvas->SizeY);

	float scale = FMath::Min(screen.Z, screen.W);
	scale *= this->m_ScaleMaster;
	scale *= this->IsViewportVertical() ? this->m_ScaleVertical : this->m_ScaleHorizontal;

	for (UHUDComponent *component : this->m_Components)
	{
		if (this->m_bPaused && !component->m_bRenderWhenPaused)
		{
			continue;
		}
		component->DrawComponent(this, origin, extent, scale);
	}

	if (this->GetCharacter()->GetHealth() <= 0.0f)
	{
		Super::DrawText(TEXT("DEAD"), FLinearColor::Red, 100, 100, this->m_Font, scale * 2.0f);
	}
}
