// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "CapturePointsComponent.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#define CP_OWNED_COLOR FLinearColor(0.0f, 0.75f, 0.0f, 0.25f)
#define CP_UNOWNED_COLOR  FLinearColor(0.8f, 0.0f, 0.0f, 0.25f)
#define CP_TEXT_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.8f)

#define CP_ARROW_COLOR FLinearColor(0.6f, 0.6f, 0.6f, 0.5f)

#define CP_MATERIAL_PARAM_ALPHA TEXT("Alpha")
#define CP_MATERIAL_PARAM_BOX_COLOR TEXT("Color")
#define CP_MATERIAL_PARAM_TEAM_COLOR TEXT("TeamColor")
#define CP_MATERIAL_PARAM_TEAM_SIZE TEXT("TeamBoxSize")
#define CP_MATERIAL_PARAM_CAPTURE_PERC TEXT("TeamCapturePerc")
#define CP_MATERIAL_PARAM_CAPTURE_COLOR TEXT("TeamCaptureColor")
#define CP_MATERIAL_PARAM_FLASH_SPEED TEXT("BoxFlashSpeed")

UCapturePointsComponent::UCapturePointsComponent() : m_Material(nullptr)
{
	this->m_OwnedColor = CP_OWNED_COLOR;
	this->m_UnownedColor = CP_UNOWNED_COLOR;
	this->m_PositionRadiusScale = 0.8f;
	this->m_bAlwaysOnScreen = true;
	this->m_BoxScale = 34.0f;
	this->m_TeamBoxUVSize = 0.12f;
	this->m_BoxAlpha = 1.0f;
	this->m_BoxFlashSpeed = 0.65f;
	this->m_ArrowAngle = 25.0f;
	this->m_ArrowTickness = 1.25f;
	this->m_ArrowDistOffset0 = 3.25f;
	this->m_ArrowDistOffset12 = 1.25f;
	this->m_ArrowColor = CP_ARROW_COLOR;
	this->m_bRenderArrow = true;
	this->m_TextColor = CP_TEXT_COLOR;
	this->m_TextScale = 0.58f;
	this->m_TextZOffset = 140.0f;
	this->m_TextShadowOffset = FVector2D(0.5f, 0.5f);
}

void UCapturePointsComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	if (this->m_Material == nullptr)
	{
		return;
	}
	ABaseGameMode *gamemode = GetGameMode(hud->GetWorld());
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
		if(!point->IsRenderHUDIconEnabled())
		{
			continue;
		}
		FString name = point->GetPointName().ToString();
		if (name.Len() != 0)
		{
			// Set the letter to the uppercase first character
			name = name.Mid(0, 1).ToUpper();
		}

		// Screen position
		FVector position = hud->Project(points[i]->GetActorLocation() + FVector(0.0f, 0.0f, this->m_TextZOffset));

		float textScale = scale * this->m_TextScale, boxScale = textScale * this->m_BoxScale;
		if (this->m_bAlwaysOnScreen)
		{
			if (position.Z <= 0.0f)
			{
				position.X = extent.X - position.X;
				position.Y = extent.Y - position.Y;

				position.X += position.X > extent.X / 2.0f ? extent.X : -extent.X;
			}

			const FVector2D position2d = FVector2D(position), vector = position2d - extent * 0.5f;
			const FVector2D max = extent * 0.5f * this->m_PositionRadiusScale;

			FVector2D ellipsePos;
			float theta = FMath::Atan2(vector.Y, vector.X);
			ellipsePos.X = (max.X * max.Y) / FMath::Sqrt(max.Y * max.Y + max.X * max.X * FMath::Pow(FMath::Tan(theta), 2.0f));
			if (theta < -PI / 2.0f || theta > PI / 2.0f)
			{
				ellipsePos.X = -ellipsePos.X;
			}
			ellipsePos.Y = ellipsePos.X * FMath::Tan(theta);
			ellipsePos += extent * 0.5f;

			if (vector.SizeSquared() > (ellipsePos - extent * 0.5f).SizeSquared())
			{
				if(this->m_bRenderArrow)
				{
					FVector2D normal = (position2d - ellipsePos).GetSafeNormal();

					float delta = FMath::Atan2(normal.Y, normal.X), deltaOffset = this->m_ArrowAngle * DEG_TO_RAD;
					FVector2D p1 = FVector2D(FMath::Cos(delta + deltaOffset), FMath::Sin(delta + deltaOffset)),
						p2 = FVector2D(FMath::Cos(delta - deltaOffset), FMath::Sin(delta - deltaOffset));

					float posOffset = boxScale * 0.5f + this->m_ArrowDistOffset12 * scale;
					p1 = ellipsePos + p1 * posOffset;
					p2 = ellipsePos + p2 * posOffset;

					FVector2D p0 = ellipsePos + (boxScale * 0.5f + this->m_ArrowDistOffset0 * scale) * normal;
					hud->DrawLine(p0.X, p0.Y, p1.X, p1.Y, this->m_ArrowColor, this->m_ArrowTickness * scale);
					hud->DrawLine(p0.X, p0.Y, p2.X, p2.Y, this->m_ArrowColor, this->m_ArrowTickness * scale);
				}
				
				position.X = ellipsePos.X;
				position.Y = ellipsePos.Y;
			}
		}
		else
		{
			if (position.Z <= 0.0f
				|| position.X < boxScale / 2.0f || position.X > extent.X - boxScale / 2.0f
				|| position.Y < boxScale / 2.0f || position.Y > extent.Y - boxScale / 2.0f)
			{
				continue;
			}
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
		FLinearColor teamColor = gamemode->GetTeamColor(point->GetOwningTeam());
		//teamColor.A = this->m_BoxAlpha;
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_ALPHA, this->m_BoxAlpha);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_FLASH_SPEED, this->m_BoxFlashSpeed);
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_BOX_COLOR, team == point->GetOwningTeam() ? this->m_OwnedColor : this->m_UnownedColor);
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_TEAM_COLOR, teamColor);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_TEAM_SIZE, this->m_TeamBoxUVSize);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_CAPTURE_PERC, point->GetCapturePercentage());
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_CAPTURE_COLOR, gamemode->GetTeamColor(point->GetCapturingTeam()));

		hud->DrawMaterialSimple(material, position.X - boxScale / 2.0f, position.Y - boxScale / 2.0f, boxScale, boxScale);

		// Draw Text
		if(name.Len() != 0)
		{
			//FLinearColor textColor = team == point->GetOwningTeam() ? FLinearColor(0.0f, 1.0f, 0.0f, 0.75f) : FLinearColor(1.0f, 0.0f, 0.0f, 0.75f);
			FLinearColor shadowColor = teamColor * 0.01f;
			FVector2D shadowOffset = this->m_TextShadowOffset * scale;

			teamColor.A = 0.9f;
			shadowColor.A = teamColor.A * 0.75f;

			float width, height;
			hud->GetTextSize(name, width, height, hud->GetFont(), textScale);
			hud->DrawText(name, shadowColor, position.X - width / 2.0f + shadowOffset.X,
				position.Y - height / 2.0f + shadowOffset.Y, hud->GetFont(), textScale);
			hud->DrawText(name, teamColor, position.X - width / 2.0f,
				position.Y - height / 2.0f, hud->GetFont(), textScale);
		}
	}
}
