// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BuildWheel.h"

#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"

#include "Event/EventManager.h"
#include "Gamemode/BaseGameMode.h"
#include "Character/PlayerCharacter.h"
#include "Construction/Prefab.h"
#include "Construction/BlockData.h"
#include "Construction/Brush/PrimaryBrush.h"
#include "Construction/Blocks/BasicBlock.h"

#define RADIUS_PERC 0.8f

#define MAX_MAT_RADIUS 0.5f
#define INNER_RADIUS 0.46f // Must not be higher than MAX_MAT_RADIUS
#define OUTER_RADIUS (MAX_MAT_RADIUS - INNER_RADIUS)

#define MENU_NAME_SCALE 0.0055f
#define MENU_NAME_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.6f)

#define SEGMENT_NAME_SCALE 0.0085f
#define SEGMENT_NAME_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.8f)

#define CENTER_CIRCLE_RADIUS_PERC 0.16f
#define CENTER_CIRCLE_INNER_RADIUS 0.4f
#define CENTER_CIRCLE_OUTER_RADIUS (MAX_MAT_RADIUS - CENTER_CIRCLE_INNER_RADIUS)

#define SEGMENT_SEPARATOR_OFFSET 0.05f

#define SELECT_COLOR FLinearColor(0.0f, 0.0f, 0.0f, 0.3f) 
#define SELECT_BORDER_COLOR FLinearColor(0.85f, 1.0f, 0.85f, 0.4f) 
#define BACKGROUND_COLOR FLinearColor(0.01f, 0.01f, 0.01f, 0.75f)

#define TEAM1_COLOR_PRIMARY FLinearColor(0.04f, 0.04f, 0.1f, 0.4f)
#define TEAM1_COLOR_SECONDARY FLinearColor(0.4f, 0.4f, 1.0f, 0.8f)

#define TEAM2_COLOR_PRIMARY FLinearColor(0.05f, 0.02f, 0.02f, 0.5f)
#define TEAM2_COLOR_SECONDARY FLinearColor(0.5f, 0.15f, 0.15f, 0.8f)

#define NAME_INNER_RADIUS TEXT("Inner Radius")
#define NAME_OUTER_RADIUS TEXT("Outer Radius")
#define NAME_PRIMARY_COLOR TEXT("Primary Color")
#define NAME_SECONDARY_COLOR TEXT("Secondary Color")
#define NAME_THETA_BEGIN TEXT("Theta Begin")
#define NAME_THETA_END TEXT("Theta End")

#define PREFAB1 TEXT("'/Game/Blueprints/Construction/Prefabs/BP_Prefab1'")
#define PREFAB2 TEXT("'/Game/Blueprints/Construction/Prefabs/BP_Prefab2'")
#define PREFAB3 TEXT("'/Game/Blueprints/Construction/Prefabs/BP_Prefab3'")
#define PREFAB4 TEXT("'/Game/Blueprints/Construction/Prefabs/BP_Prefab4'")
#define PREFAB5 TEXT("'/Game/Blueprints/Construction/Prefabs/BP_Prefab5'")

#define SELECT_MATERIAL TEXT("Material'/Game/Materials/HUD/M_BuildWheelSelection.M_BuildWheelSelection'")
#define BUILD_WHEEL_MATERIAL TEXT("Material'/Game/Materials/HUD/M_BuildWheel.M_BuildWheel'")

UBuildWheel::UBuildWheel() : m_bIsVisible(false)
{
	static ConstructorHelpers::FClassFinder<APrefab> Prefab1(PREFAB1);
	if (Prefab1.Succeeded())
	{
		this->m_Prefabs.Add(Prefab1.Class);
	}
	static ConstructorHelpers::FClassFinder<APrefab> Prefab2(PREFAB2);
	if (Prefab2.Succeeded())
	{
		this->m_Prefabs.Add(Prefab2.Class);
	}
	static ConstructorHelpers::FClassFinder<APrefab> Prefab3(PREFAB3);
	if (Prefab3.Succeeded())
	{
		this->m_Prefabs.Add(Prefab3.Class);
	}
	static ConstructorHelpers::FClassFinder<APrefab> Prefab4(PREFAB4);
	if (Prefab4.Succeeded())
	{
		this->m_Prefabs.Add(Prefab4.Class);
	}
	static ConstructorHelpers::FClassFinder<APrefab> Prefab5(PREFAB5);
	if (Prefab5.Succeeded())
	{
		this->m_Prefabs.Add(Prefab5.Class);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WheelMaterial(BUILD_WHEEL_MATERIAL);
	if (WheelMaterial.Succeeded())
	{
		UMaterialInstanceDynamic *material;

		material = UMaterialInstanceDynamic::Create(WheelMaterial.Object, Super::GetOuter());
		material->SetScalarParameterValue(NAME_INNER_RADIUS, INNER_RADIUS);
		material->SetScalarParameterValue(NAME_OUTER_RADIUS, OUTER_RADIUS);

		this->m_WheelMaterial = material;

		material = UMaterialInstanceDynamic::Create(WheelMaterial.Object, Super::GetOuter());
		material->SetScalarParameterValue(NAME_INNER_RADIUS, CENTER_CIRCLE_INNER_RADIUS);
		material->SetScalarParameterValue(NAME_OUTER_RADIUS, CENTER_CIRCLE_OUTER_RADIUS);

		this->m_InnerWheelMaterial = material;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SelectMaterial(SELECT_MATERIAL);
	if (SelectMaterial.Succeeded())
	{
		UMaterialInstanceDynamic *material = UMaterialInstanceDynamic::Create(SelectMaterial.Object, Super::GetOuter());
		material->SetVectorParameterValue(NAME_PRIMARY_COLOR, SELECT_COLOR);
		material->SetVectorParameterValue(NAME_SECONDARY_COLOR, SELECT_BORDER_COLOR);
		material->SetScalarParameterValue(NAME_INNER_RADIUS, 0.0127f); //TODO math this
		material->SetScalarParameterValue(NAME_OUTER_RADIUS, INNER_RADIUS);

		this->m_SelectMaterial = material;
	}
}

void UBuildWheel::UpdateSelectMaterial() const
{
	UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_SelectMaterial);
	if (material != nullptr && this->m_SelectData.index != -1 && this->m_SelectData.updated)
	{
		material->SetScalarParameterValue(NAME_THETA_BEGIN, this->m_SelectData.theta_begin);
		material->SetScalarParameterValue(NAME_THETA_END, this->m_SelectData.theta_end);
	}
}

void UBuildWheel::SetSelected(float radians)
{
	radians = PI2 - FMath::Fmod(radians + PI2, PI2);

	WheelMenu& active = this->GetActiveMenu();
	float delta = PI2 / active.m_Segments.size();
	this->SetSelected(int(radians / delta) % int(active.m_Segments.size()));
}

void UBuildWheel::Select(APlayerCharacter *character)
{
	if (character == nullptr)
	{
		return;
	}
	if (this->m_SelectData.index != -1)
	{
		auto action = this->GetActiveMenu().m_Segments[this->m_SelectData.index].action;
		if (action != nullptr && !action(character))
		{
			this->SetVisible(false);
		}
	}
}

void UBuildWheel::Back()
{
	if (this->m_MenuStack.size() > 1)
	{
		this->m_MenuStack.pop();

		this->m_SelectData.index = -1;
		this->m_SelectData.updated = false;
	}
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
	if (material != nullptr)
	{
		material->SetVectorParameterValue(NAME_PRIMARY_COLOR, this->m_PrimaryColor * 0.84f);
		material->SetVectorParameterValue(NAME_SECONDARY_COLOR, this->m_SecondaryColor * 0.84f);
	}
}

WheelMenu UBuildWheel::CreateWheelMenu()
{
	WheelMenu root(TEXT("menu"));
	{
		//root.AddSegment({ TEXT("Events"), [this](APlayerCharacter *character)
		//{
		//	WheelMenu eventMenu(TEXT("events"));
		//	{
		//		std::function<bool(EGameEvent, FName, const int&)> trigger =
		//			[this, character](EGameEvent type, FName id, const int& cost)
		//		{
		//			UBlockData *data = character->GetPrimaryBrush()->GetBlockData(id);
		//			if (data == nullptr || data->GetCount() < cost)
		//			{
		//				return true;
		//			}
		//			data->SetCount(character->GetPrimaryBrush(), data->GetCount() - cost);
		//			ABaseGameMode *gamemode = Cast<ABaseGameMode>(character->GetWorld()->GetAuthGameMode());
		//			if (gamemode != nullptr && gamemode->GetEventManager())
		//			{
		//				gamemode->GetEventManager()->TriggerEvent(type);
		//			}
		//			return false;
		//		};
		//		eventMenu.AddSegment({ TEXT("Enable Low Gravity"), [this, trigger](APlayerCharacter *character)
		//		{
		//			return trigger(EGameEvent::LowGravity, ID_BASIC_BLOCK, 1);
		//		} });
		//		eventMenu.AddSegment({ TEXT("Enable Lava"), [this, trigger](APlayerCharacter *character)
		//		{
		//			return trigger(EGameEvent::FloorIsLava, ID_BASIC_BLOCK, 1);
		//		} });
		//	}
		//	return this->OpenMenu(eventMenu);
		//} });

		root.AddSegment({ TEXT("Power-up"), [this](APlayerCharacter *character)
		{
			WheelMenu powerMenu(TEXT("prefab"));
			{
				powerMenu.AddSegment({ TEXT("none"), [this](APlayerCharacter *character)
				{
					character->GetPrimaryBrush()->SetPrefab(nullptr);
					return false;
				} });
				powerMenu.AddSegment({ TEXT("prefab1"), [this](APlayerCharacter *character)
				{
					character->GetPrimaryBrush()->SetPrefab(this->m_Prefabs[0]);
					return false;
				} });
				powerMenu.AddSegment({ TEXT("prefab2"), [this](APlayerCharacter *character)
				{
					character->GetPrimaryBrush()->SetPrefab(this->m_Prefabs[1]);
					return false;
				} });
				powerMenu.AddSegment({ TEXT("prefab3"), [this](APlayerCharacter *character)
				{
					character->GetPrimaryBrush()->SetPrefab(this->m_Prefabs[2]);
					return false;
				} });
				powerMenu.AddSegment({ TEXT("prefab4"), [this](APlayerCharacter *character)
				{
					character->GetPrimaryBrush()->SetPrefab(this->m_Prefabs[3]);
					return false;
				} });
				powerMenu.AddSegment({ TEXT("prefab5"), [this](APlayerCharacter *character)
				{
					character->GetPrimaryBrush()->SetPrefab(this->m_Prefabs[4]);
					return false;
				} });
			}
			return this->OpenMenu(powerMenu);
		} });

		root.AddSegment({ TEXT("Upgrades"), [this](APlayerCharacter *character)
		{
			WheelMenu upgradeMenu(TEXT("upgrade"));
			{
				upgradeMenu.AddSegment({ TEXT("Increase Stamina"), [this](APlayerCharacter *character)
				{
					return false;
				} });
				upgradeMenu.AddSegment({ TEXT("Increase Health"), [this](APlayerCharacter *character)
				{
					return false;
				} });
			}
			return this->OpenMenu(upgradeMenu);
		} });
	}
	return root;
}

void UBuildWheel::Render(AHUD* display, FVector4& screen)
{
	if (!this->IsVisible())
	{
		return;
	}
	float originX = screen.X + screen.Z / 2.0f, originY = screen.Y + screen.W / 2.0f;
	float radius = FMath::Min(screen.Z * RADIUS_PERC, screen.W * RADIUS_PERC) / 2.0f;
	float centerRadius = radius * CENTER_CIRCLE_RADIUS_PERC;

	display->DrawRect(BACKGROUND_COLOR, screen.X, screen.Y, screen.Z, screen.W);

	display->DrawMaterial(this->m_WheelMaterial, originX - radius, originY - radius,
		radius * 2.0f, radius * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	FVector2D size;
	WheelMenu& activeMenu = this->GetActiveMenu();

	// Draw lines and text
	float inner = radius - (radius / MAX_MAT_RADIUS) * (OUTER_RADIUS / 2.0f);
	for (unsigned int i = 0; i < activeMenu.m_Segments.size(); i++)
	{
		WheelMenuSegment& segment = activeMenu.m_Segments[i];

		float delta = PI2 / activeMenu.m_Segments.size();
		float c = FMath::Cos(i * delta), s = FMath::Sin(i * delta);

		float offsetX = c * (inner - centerRadius) * SEGMENT_SEPARATOR_OFFSET,
			offsetY = s * (inner - centerRadius) * SEGMENT_SEPARATOR_OFFSET;

		// The line
		display->DrawLine(originX + c * centerRadius + offsetX, originY + s * centerRadius + offsetY,
			originX + c * inner - offsetX, originY + s * inner - offsetY, this->m_PrimaryColor, 2);

		// The text
		c = FMath::Cos((i + 0.5f) * delta), s = FMath::Sin((i + 0.5f) * delta);
		display->GetTextSize(segment.name, size.X, size.Y, nullptr, radius * SEGMENT_NAME_SCALE);
		display->DrawText(segment.name, SEGMENT_NAME_COLOR,
			originX + c * centerRadius + c * (inner - centerRadius) * 0.5f - size.X / 2.0f,
			originY + s * centerRadius + s * (inner - centerRadius) * 0.5f - size.Y / 2.0f, nullptr, radius * SEGMENT_NAME_SCALE);

		// Update select data if required
		if (this->m_SelectData.index == i && !this->m_SelectData.updated)
		{
			this->m_SelectData.theta_begin = i * delta;
			this->m_SelectData.theta_end = (i + 1) * delta;
			this->m_SelectData.updated = true;

			this->UpdateSelectMaterial();
		}
	}

	// Draw center circle
	display->DrawMaterial(this->m_InnerWheelMaterial, originX - centerRadius, originY - centerRadius,
		centerRadius * 2.0f, centerRadius * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Draw selection
	if (this->m_SelectData.index != -1 && this->m_SelectData.updated)
	{
		display->DrawMaterial(this->m_SelectMaterial, originX - radius, originY - radius,
			radius * 2.0f, radius * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	// Draw menu name
	display->GetTextSize(activeMenu.m_Name, size.X, size.Y, nullptr, radius * MENU_NAME_SCALE);
	display->DrawText(activeMenu.m_Name, MENU_NAME_COLOR,
		originX - size.X / 2.0f, originY - size.Y / 2.0f, nullptr, radius * MENU_NAME_SCALE);
}
