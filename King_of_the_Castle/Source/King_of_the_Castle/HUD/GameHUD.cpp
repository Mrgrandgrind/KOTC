// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "Engine/Font.h"
#include "Engine/Canvas.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#define CROSSHAIR_SIZE 0.016f // Multiplier of min(width, height)
#define CROSSHAIR_COLOR FLinearColor(0.25f, 0.75f, 0.75f, 0.5f)

#define WIDGET_HUD_LOCATION TEXT("/Game/Blueprints/BP_HUD")

#define FONT_LOCATION TEXT("Font'/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField'")

AGameHUD::AGameHUD() : m_WidgetHUD(nullptr), m_bCrosshairVisible(false)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetHUD(WIDGET_HUD_LOCATION);
	if (WidgetHUD.Succeeded())
	{
		this->m_WidgetHUDClass = WidgetHUD.Class;
	}

	static ConstructorHelpers::FObjectFinder<UFont> Font(FONT_LOCATION);
	if (Font.Succeeded())
	{
		this->m_Font = Font.Object;
	}

	this->m_BuildWheel = UObject::CreateDefaultSubobject<UBuildWheel>(TEXT("BuildWheel"));
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	if (this->m_WidgetHUDClass != nullptr)
	{
		this->m_WidgetHUD = CreateWidget<UUserWidget>(Super::GetOwningPlayerController(), this->m_WidgetHUDClass);
		this->m_WidgetHUD->AddToPlayerScreen();
	}
}

void AGameHUD::DrawHUD()
{
	Super::DrawHUD();
	
	FVector4 screen = FVector4(Super::Canvas->OrgX, Super::Canvas->OrgY, Super::Canvas->SizeX, Super::Canvas->SizeY);
	this->m_BuildWheel->Render(this, screen);

	if(this->m_bCrosshairVisible)
	{
		const float& size = FMath::RoundToFloat(FMath::Min(screen.Z, screen.W) * CROSSHAIR_SIZE);
		Super::DrawRect(CROSSHAIR_COLOR, screen.X + screen.Z / 2.0f - size / 2.0f,
			screen.Y + screen.W / 2.0f - size / 2.0f, size, size);
	}
}
