// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "Engine/Font.h"
#include "Engine/Canvas.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#define FONT_LOCATION TEXT("Font'/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField'")

AGameHUD::AGameHUD() : m_Widget(nullptr)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> Widget(TEXT("/Game/Blueprints/BP_HUD"));
	if (Widget.Succeeded())
	{
		this->m_WidgetClass = Widget.Class;
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

	if (this->m_WidgetClass != nullptr)
	{
		this->m_Widget = CreateWidget<UUserWidget>(Super::GetOwningPlayerController(), this->m_WidgetClass);
		this->m_Widget->AddToPlayerScreen();
	}
}

void AGameHUD::DrawHUD()
{
	Super::DrawHUD();
	
	FVector4 screen = FVector4(Super::Canvas->OrgX, Super::Canvas->OrgY, Super::Canvas->SizeX, Super::Canvas->SizeY);
	this->m_BuildWheel->Render(this, screen);
}
