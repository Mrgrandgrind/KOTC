// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "CapturePointsComponent.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#define CP_OWNED_COLOR FLinearColor(0.0f, 0.75f, 0.0f, 0.25f)
#define CP_UNOWNED_COLOR  FLinearColor(0.8f, 0.0f, 0.0f, 0.25f)
#define CP_TEXT_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.8f)

#define CP_MATERIAL_PARAM_ALPHA TEXT("Alpha")
#define CP_MATERIAL_PARAM_BOX_COLOR TEXT("Color")
#define CP_MATERIAL_PARAM_TEAM_COLOR TEXT("TeamColor")
#define CP_MATERIAL_PARAM_TEAM_SIZE TEXT("TeamBoxSize")
#define CP_MATERIAL_PARAM_CAPTURE_PERC TEXT("TeamCapturePerc")
#define CP_MATERIAL_PARAM_CAPTURE_COLOR TEXT("TeamCaptureColor")
#define CP_MATERIAL_PARAM_FLASH_SPEED TEXT("BoxFlashSpeed")

UCapturePointsComponent::UCapturePointsComponent()
{
	this->m_OwnedColor = CP_OWNED_COLOR;
	this->m_UnownedColor = CP_UNOWNED_COLOR;
	this->m_BoxScale = 31.0f;
	this->m_TeamBoxUVSize = 0.12f;
	this->m_BoxAlpha = 0.5f;
	this->m_BoxFlashSpeed = 0.65f;
	this->m_TextColor = CP_TEXT_COLOR;
	this->m_TextScale = 0.58f;
	this->m_TextZOffset = 140.0f;
}

void UCapturePointsComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	if (this->m_Material == nullptr)
	{
		return;
	}
	ABaseGameMode *gamemode = GetGameMode<ABaseGameMode>(hud->GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	const int& team = hud->GetCharacter()->GetTeam();

	TArray<AActor*> points;
	UGameplayStatics::GetAllActorsOfClass(hud->GetWorld(), ACapturePoint::StaticClass(), points);

	//for (AActor *actor : points)
	for (int i = 0; i < points.Num(); i++)
	{
		ACapturePoint *point = Cast<ACapturePoint>(points[i]);
		FString name = point->GetPointName().ToString();
		// Don't render if the point has no name
		if (name.Len() == 0)
		{
			continue;
		}
		// Set the letter to the uppercase first character
		name = name.Mid(0, 1).ToUpper();

		// Screen position
		FVector position = hud->Project(points[i]->GetActorLocation() + FVector(0.0f, 0.0f, this->m_TextZOffset));

		float textScale = scale * this->m_TextScale, boxScale = textScale * this->m_BoxScale;
		if (position.Z <= 0.0f
			|| position.X < boxScale / 2.0f || position.X > extent.X - boxScale / 2.0f
			|| position.Y < boxScale / 2.0f || position.Y > extent.Y - boxScale / 2.0f)
		{
			continue;
		}
		// Draw background box
		if (!this->m_Materials.Contains(point))
		{
			this->m_Materials.Add(point, UMaterialInstanceDynamic::Create(this->m_Material, Super::GetOuter()));
		}
		UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_Materials[point]);
		if (material == nullptr)
		{
			continue;
		}
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_ALPHA, this->m_BoxAlpha);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_FLASH_SPEED, this->m_BoxFlashSpeed);
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_BOX_COLOR, team == point->GetOwningTeam() ? this->m_OwnedColor : this->m_UnownedColor);
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_TEAM_COLOR, gamemode->GetTeamColor(point->GetOwningTeam()));
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_TEAM_SIZE, this->m_TeamBoxUVSize);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_CAPTURE_PERC, point->GetCapturePercentage());
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_CAPTURE_COLOR, gamemode->GetTeamColor(point->GetCapturingTeam()));

		//position.X = FMath::Max(boxScale, FMath::Min(screen.Z - boxScale, position.X));
		//position.Y = FMath::Max(boxScale, FMath::Min(screen.W - boxScale, position.Y));

		hud->DrawMaterialSimple(material, position.X - boxScale / 2.0f, position.Y - boxScale / 2.0f, boxScale, boxScale);

		// Draw Text
		float width, height;
		hud->GetTextSize(name, width, height, hud->GetFont(), textScale);
		hud->DrawText(name, this->m_TextColor, position.X - width / 2.0f,
			position.Y - height / 2.0f, hud->GetFont(), textScale);
	}
}