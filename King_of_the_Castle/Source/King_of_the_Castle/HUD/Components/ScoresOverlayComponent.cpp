// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "ScoresOverlayComponent.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints.h"

UScoresOverlayComponent::UScoresOverlayComponent()
{
	this->m_BarAlpha = 0.75f;
	this->m_BarMoveSpeed = 500.0f;
	this->m_GraphSize = FVector2D(180.0f, 100.0f);

	Super::m_bRender = false;
}

void UScoresOverlayComponent::SetVisible(const bool& visible)
{
	Super::SetVisible(visible);

	if (visible)
	{
		int count = this->m_Scores.Num();
		this->m_Scores.Empty();
		this->m_Scores.Init(0.0f, count);
	}
}

void UScoresOverlayComponent::Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(hud->GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	float x, y;
	float width, height;

	float axisSize = scale * 3.0f, axisExtra = scale * 10.0f;
	float graphWidth = this->m_GraphSize.X * scale, graphHeight = this->m_GraphSize.Y * scale;

	float bkgWidth = graphWidth * 1.3f, bkgHeight = graphHeight * 1.3f;
	hud->DrawRect(FLinearColor(0.01f, 0.01f, 0.01f, 0.5f), 
		origin.X + extent.X / 2.0f - bkgWidth / 2.0f, 
		origin.Y + extent.Y / 2.0f - bkgHeight / 2.0f, bkgWidth, bkgHeight);

	// Title
	FString name = gamemode->GetGameModeName().ToString();
	hud->GetTextSize(name, width, height, hud->GetFont(), scale * 0.8f);

	x = origin.X + extent.X / 2.0f - width / 2.0f;
	y = origin.Y + extent.Y * 0.15f - height / 2.0f;
	
	float offset = scale * 8.0f;
	hud->DrawRect(FLinearColor(0.01f, 0.01f, 0.01f, 0.5f), x - offset / 2.0f, y - offset / 4.0f, 
		width + offset, (origin.Y + extent.Y / 2.0f - bkgHeight / 2.0f) - (y - offset / 4.0f));
	hud->DrawText(name, FLinearColor::White, x, y, hud->GetFont(), scale * 0.8f);

	FLinearColor teamColor = gamemode->GetTeamColor(hud->GetCharacter()->GetTeam());
	teamColor.A = this->m_BarAlpha;

	// X Axis
	hud->DrawRect(teamColor, origin.X + extent.X / 2.0f - graphWidth / 2.0f - axisExtra,
		origin.Y + extent.Y / 2.0f + graphHeight / 2.0f, graphWidth + axisExtra * 2.0f, axisSize);
	// Y Axis
	hud->DrawRect(teamColor, origin.X + extent.X / 2.0f - graphWidth / 2.0f,
		origin.Y + extent.Y / 2.0f - graphHeight / 2.0f, axisSize, graphHeight + axisExtra);

	// Init scores if not already done
	if (this->m_Scores.Num() != gamemode->GetTeamCount())
	{
		this->m_Scores.Empty();
		this->m_Scores.Init(0.0f, gamemode->GetTeamCount());
	}

	y = origin.Y + extent.Y / 2.0f + graphHeight / 2.0f;

	float barGoalHeight = scale * 20.0f;
	float barGap = scale * 20.0f, barSize = (graphWidth - barGap - barGap * (gamemode->GetTeamCount() - 1)) / gamemode->GetTeamCount();
	for (int i = 0; i < gamemode->GetTeamCount(); i++)
	{
		FLinearColor color = gamemode->GetTeamColor(i + 1);
		color.A = this->m_BarAlpha;

		//int score = (1.0f - float(i) / gamemode->GetTeamCount()) * gamemode->GetWinScore(); // (gamemode->GetScore(i + 1) / gamemode->GetWinScore());
		int score = gamemode->GetScore(i + 1);
		score = FMath::FInterpConstantTo(this->m_Scores[i], score, hud->GetWorld()->GetDeltaSeconds(), this->m_BarMoveSpeed);
		this->m_Scores[i] = score;

		float barHeight = (graphHeight - barGoalHeight) * score / gamemode->GetWinScore();
		x = origin.X + extent.X / 2.0f - graphWidth / 2.0f + axisSize + barGap / 2.0f + barSize * i + barGap * i;
		hud->DrawRect(color, x, y, barSize, -barHeight);

		//FString teamText = FString::Printf(TEXT("Player %d"), i + 1);
		//hud->GetTextSize(teamText, width, height, hud->GetFont(), 0.38f * scale);
		//hud->DrawText(teamText, color, x + barSize / 2.0f - width / 2.0f, y + axisSize * 1.5f, hud->GetFont(), 0.38f * scale);
	}

	// Draw goal line
	float goalLineHeight = scale * 1.3f, goalLineGap = scale * 4.0f;
	float gx = origin.X + extent.X / 2.0f - graphWidth / 2.0f - axisSize;
	float gy = origin.Y + extent.Y / 2.0f - graphHeight / 2.0f + barGoalHeight - goalLineHeight;
	for (float rgx = gx; rgx < gx + graphWidth + axisSize; rgx += goalLineGap * 2.0f)
	{
		hud->DrawRect(FLinearColor::White, rgx, gy, goalLineGap, goalLineHeight);
	}
	FString goalText = FString("GOAL");
	hud->GetTextSize(goalText, width, height, hud->GetFont(), scale * 0.3f);
	hud->DrawText(goalText, FLinearColor::White, gx - width * 1.1f, gy - height / 2.0f + goalLineHeight / 2.0f, hud->GetFont(), scale * 0.3f);
}