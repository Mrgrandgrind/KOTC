// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "Engine/Font.h"
#include "Engine/Canvas.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#define WIDGET_HUD_LOCATION TEXT("/Game/Blueprints/BP_HUD")

#define FONT_LOCATION TEXT("Font'/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField'")

AGameHUD::AGameHUD() : m_WidgetHUD(nullptr)
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
}
