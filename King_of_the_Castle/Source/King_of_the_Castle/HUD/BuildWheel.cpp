// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BuildWheel.h"

#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"

#define RADIUS_PERC 0.8f

#define MAX_MAT_RADIUS 0.5f
#define INNER_RADIUS 0.46f // Must not be higher than MAX_MAT_RADIUS
#define OUTER_RADIUS (MAX_MAT_RADIUS - INNER_RADIUS)

#define MENU_NAME_SCALE 1.2f
#define MENU_NAME_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.6f)

#define SEGMENT_NAME_SCALE 1.8f
#define SEGMENT_NAME_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.8f)

#define CENTER_CIRCLE_RADIUS_PERC 0.16f
#define CENTER_CIRCLE_INNER_RADIUS 0.4f
#define CENTER_CIRCLE_OUTER_RADIUS (MAX_MAT_RADIUS - CENTER_CIRCLE_INNER_RADIUS)

#define SEGMENT_SEPARATOR_OFFSET 0.08f

#define BACKGROUND_COLOR FLinearColor(0.01f, 0.01f, 0.01f, 0.75f)

#define TEAM1_COLOR_PRIMARY FLinearColor(0.04f, 0.04f, 0.1f, 0.4f)
#define TEAM1_COLOR_SECONDARY FLinearColor(0.4f, 0.4f, 1.0f, 0.8f)

#define TEAM2_COLOR_PRIMARY FLinearColor(0.05f, 0.02f, 0.02f, 0.5f)
#define TEAM2_COLOR_SECONDARY FLinearColor(0.5f, 0.15f, 0.15f, 0.8f)

#define NAME_INNER_RADIUS TEXT("Inner Radius")
#define NAME_OUTER_RADIUS TEXT("Outer Radius")
#define NAME_PRIMARY_COLOR TEXT("Primary Color")
#define NAME_SECONDARY_COLOR TEXT("Secondary Color")
#define BUILD_WHEEL_MATERIAL TEXT("Material'/Game/Materials/HUD/M_BuildWheel.M_BuildWheel'")

UBuildWheel::UBuildWheel() : m_bIsVisible(false)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(BUILD_WHEEL_MATERIAL);
	if (Material.Succeeded())
	{
		UMaterialInstanceDynamic *material;

		material = UMaterialInstanceDynamic::Create(Material.Object, Super::GetOuter());
		material->SetScalarParameterValue(NAME_INNER_RADIUS, INNER_RADIUS);
		material->SetScalarParameterValue(NAME_OUTER_RADIUS, OUTER_RADIUS);

		this->m_WheelMaterial = material;

		material = UMaterialInstanceDynamic::Create(Material.Object, Super::GetOuter());
		material->SetScalarParameterValue(NAME_INNER_RADIUS, CENTER_CIRCLE_INNER_RADIUS);
		material->SetScalarParameterValue(NAME_OUTER_RADIUS, CENTER_CIRCLE_OUTER_RADIUS);

		this->m_InnerWheelMaterial = material;
	}
	this->m_RootMenu.m_Segments.Add({ TEXT("Turrets") });
	this->m_RootMenu.m_Segments.Add({ TEXT("Gadgets") });
	this->m_RootMenu.m_Segments.Add({ TEXT("Walls") });
	this->m_RootMenu.m_Segments.Add({ TEXT("Upgrades") });

	this->m_CurrentMenu = &this->m_RootMenu;
}

void UBuildWheel::SetTeam(const int& team)
{
	UMaterialInstanceDynamic *material;
	material = Cast<UMaterialInstanceDynamic>(this->m_WheelMaterial);
	if (material != nullptr)
	{
		this->m_PrimaryColor = team <= 1 ? TEAM1_COLOR_PRIMARY : TEAM2_COLOR_PRIMARY;
		this->m_SecondaryColor = team <= 1 ? TEAM1_COLOR_SECONDARY : TEAM2_COLOR_SECONDARY;

		material->SetVectorParameterValue(NAME_PRIMARY_COLOR, this->m_PrimaryColor);
		material->SetVectorParameterValue(NAME_SECONDARY_COLOR, this->m_SecondaryColor);
	}
	material = Cast<UMaterialInstanceDynamic>(this->m_InnerWheelMaterial);
	if(material != nullptr)
	{
		material->SetVectorParameterValue(NAME_PRIMARY_COLOR, this->m_PrimaryColor * 0.84f);
		material->SetVectorParameterValue(NAME_SECONDARY_COLOR, this->m_SecondaryColor * 0.84f);
	}
}

void UBuildWheel::Render(AHUD* display, FVector2D& screen) const
{
	if (!this->IsVisible())
	{
		return;
	}
	float originX = screen.X / 2.0f, originY = screen.Y / 2.0f;
	float radius = FMath::Min(screen.X * RADIUS_PERC, screen.Y * RADIUS_PERC) / 2.0f;
	float centerRadius = radius * CENTER_CIRCLE_RADIUS_PERC;

	display->DrawRect(BACKGROUND_COLOR, 0.0f, 0.0f, screen.X, screen.Y);

	display->DrawMaterial(this->m_WheelMaterial, originX - radius, originY - radius,
		radius * 2.0f, radius * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	if(this->m_CurrentMenu != nullptr)
	{
		FVector2D size;

		// Draw lines and text
		float inner = radius - (radius / MAX_MAT_RADIUS) * (OUTER_RADIUS / 2.0f);
		for(int i = 0; i < this->m_CurrentMenu->m_Segments.Num(); i++)
		{
			WheelMenuSegment& segment = this->m_CurrentMenu->m_Segments[i];

			float delta = PI2 / this->m_CurrentMenu->m_Segments.Num();
			float c = FMath::Cos(i * delta), s = FMath::Sin(i * delta);

			float offsetX = c * (inner - centerRadius) * SEGMENT_SEPARATOR_OFFSET,
				offsetY = s * (inner - centerRadius) * SEGMENT_SEPARATOR_OFFSET;

			// The line
			display->DrawLine(originX + c * centerRadius + offsetX, originY + s * centerRadius + offsetY,
				originX + c * inner - offsetX, originY + s * inner - offsetY, this->m_PrimaryColor, 2);

			// The text
			c = FMath::Cos((i + 0.5f) * delta), s = FMath::Sin((i + 0.5f) * delta);
			display->GetTextSize(segment.m_Name, size.X, size.Y, nullptr, SEGMENT_NAME_SCALE);
			display->DrawText(segment.m_Name, SEGMENT_NAME_COLOR,
				originX + c * centerRadius + c * (inner - centerRadius) * 0.5f - size.X / 2.0f,
				originY + s * centerRadius + s * (inner - centerRadius) * 0.5f - size.Y / 2.0f, nullptr, SEGMENT_NAME_SCALE);
		}

		// Draw center circle
		display->DrawMaterial(this->m_InnerWheelMaterial, originX - centerRadius, originY - centerRadius,
			centerRadius * 2.0f, centerRadius * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Draw menu name
		display->GetTextSize(this->m_CurrentMenu->m_Name, size.X, size.Y, nullptr, MENU_NAME_SCALE);
		display->DrawText(this->m_CurrentMenu->m_Name, MENU_NAME_COLOR,
			originX - size.X / 2.0f, originY - size.Y / 2.0f, nullptr, MENU_NAME_SCALE);
	}
}
