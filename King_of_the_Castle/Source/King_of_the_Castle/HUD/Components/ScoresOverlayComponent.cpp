// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "ScoresOverlayComponent.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints.h"

#define TITLE_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.75f)
#define GOAL_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.75f)

UScoresOverlayComponent::UScoresOverlayComponent() : m_TitleText(TEXT(""))
{
	this->m_TitleBoxScale = 8.0f;
	this->m_TitleTextScale = 0.8f;
	this->m_TitleTextColor = TITLE_COLOR;

	this->m_BarAlpha = 0.75f;
	this->m_BarMoveSpeed = 125.0f;

	this->m_GoalLineHeight = 1.3f;
	this->m_GoalHeightOffset = 20.0f;
	this->m_GoalTextScale = 0.3f;
	this->m_GoalColor = GOAL_COLOR;

	this->m_GraphYIncrements = 50.0f;
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

void UScoresOverlayComponent::DrawDashedRect(class AGameHUD *hud, const FLinearColor& color, 
	const float& x, const float& y, const float& width, const float& height, const float& gap)
{
	for (float x2 = x; x2 < x + width; x2 += gap * 2.0f)
	{
		hud->DrawRect(color, x2, y - height / 2.0f, gap, height);
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
	float goalHeight = graphHeight - this->m_GoalHeightOffset * scale;

	float bkgWidth = graphWidth * 1.3f, bkgHeight = graphHeight * 1.3f;
	hud->DrawRect(FLinearColor(0.01f, 0.01f, 0.01f, 0.4f), 
		origin.X + extent.X / 2.0f - bkgWidth / 2.0f, 
		origin.Y + extent.Y / 2.0f - bkgHeight / 2.0f, bkgWidth, bkgHeight);

	// Title
	FString name = this->m_TitleText.IsEmpty() ? gamemode->GetGameModeName().ToString() : this->m_TitleText;
	hud->GetTextSize(name, width, height, hud->GetFont(), this->m_TitleTextScale * scale);

	x = origin.X + extent.X / 2.0f - width / 2.0f;
	y = origin.Y + extent.Y / 2.0f - bkgHeight / 2.0f - height;

	float titleOffset = this->m_TitleBoxScale * scale;
	hud->DrawRect(FLinearColor(0.01f, 0.01f, 0.01f, 0.4f), x - titleOffset / 2.0f, y - titleOffset / 4.0f,
		width + titleOffset, (origin.Y + extent.Y / 2.0f - bkgHeight / 2.0f) - (y - titleOffset / 4.0f));

	if (name.StartsWith(TEXT("Player")) && name.Len() >= 8)
	{
		FString playerSub = name.Mid(0, 8), restSub = name.Mid(8), teamText = name.Mid(7, 1);
		FLinearColor color = gamemode->GetTeamColor(FCString::Atoi(*teamText));
		color.A = this->m_TitleTextColor.A;

		float w2, h2;
		hud->GetTextSize(playerSub, w2, h2, hud->GetFont(), this->m_TitleTextScale * scale);

		hud->DrawText(playerSub, color, x, y, hud->GetFont(), this->m_TitleTextScale * scale);
		hud->DrawText(restSub, this->m_TitleTextColor, x + w2, y, hud->GetFont(), this->m_TitleTextScale * scale);
	}
	else
	{
		hud->DrawText(name, this->m_TitleTextColor, x, y, hud->GetFont(), this->m_TitleTextScale * scale);
	}

	FLinearColor teamColor = gamemode->GetTeamColor(hud->GetCharacter()->GetTeam());
	teamColor.A = this->m_BarAlpha;

	x = origin.X + extent.X / 2.0f - graphWidth / 2.0f;
	y = origin.Y + extent.Y / 2.0f - graphHeight / 2.0f;

	// Axis
	hud->DrawRect(teamColor, x, y, axisSize, graphHeight + axisExtra);

	y = origin.Y + extent.Y / 2.0f + graphHeight / 2.0f;
	hud->DrawRect(teamColor, x - axisExtra, y, graphWidth + axisExtra * 2.0f, axisSize);

	// Y-axis increments
	float yIncrement = goalHeight * (this->m_GraphYIncrements / gamemode->GetWinScore());
	width = -axisExtra * 0.5f;
	height = axisSize * 0.5f;
	for (float y2 = y - yIncrement; y2 > y - goalHeight; y2 -= yIncrement)
	{
		hud->DrawRect(teamColor, x, y2 - height / 2.0f, width, height);
	}

	// Init scores if not already done
	if (this->m_Scores.Num() != gamemode->GetTeamCount())
	{
		this->m_Scores.Empty();
		this->m_Scores.Init(0.0f, gamemode->GetTeamCount());
	}

	y = origin.Y + extent.Y / 2.0f + graphHeight / 2.0f;

	float barGap = scale * 20.0f, barSize = (graphWidth - barGap - barGap * (gamemode->GetTeamCount() - 1)) / gamemode->GetTeamCount();

	// Draw goal line
	float goalLineGap = scale * 4.0f, goalLineHeight = this->m_GoalLineHeight * scale;
	float gx = origin.X + extent.X / 2.0f - graphWidth / 2.0f - axisSize;
	float gy = origin.Y + extent.Y / 2.0f - graphHeight / 2.0f + this->m_GoalHeightOffset * scale - goalLineHeight / 2.0f;
	this->DrawDashedRect(hud, this->m_GoalColor, gx, gy, graphWidth + axisSize, goalLineHeight, goalLineGap);

	FString goalText = FString("GOAL");
	hud->GetTextSize(goalText, width, height, hud->GetFont(), this->m_GoalTextScale * scale);
	hud->DrawText(goalText, this->m_GoalColor, gx - width * 1.1f, 
		gy - height / 2.0f + goalLineHeight / 2.0f, hud->GetFont(), this->m_GoalTextScale * scale);

	FString winText = FString::Printf(TEXT("%d"), int(gamemode->GetWinScore()));
	float winWidth, winHeight;
	hud->GetTextSize(winText, winWidth, winHeight, hud->GetFont(), this->m_GoalTextScale * scale);
	hud->DrawText(winText, this->m_GoalColor, gx - width * 1.1f + width / 2.0f - winWidth / 2.0f, 
		gy - height / 2.0f + goalLineHeight / 2.0f + height, hud->GetFont(), this->m_GoalTextScale * scale);

	// Draw score bars
	for (int i = 0; i < gamemode->GetTeamCount(); i++)
	{
		FLinearColor color = gamemode->GetTeamColor(i + 1);
		color.A = this->m_BarAlpha;

		//int score = (1.0f - float(i) / gamemode->GetTeamCount()) * gamemode->GetWinScore(); // (gamemode->GetScore(i + 1) / gamemode->GetWinScore());
		float score = float(gamemode->GetScore(i + 1));
		score = FMath::FInterpConstantTo(this->m_Scores[i], score, hud->GetWorld()->GetDeltaSeconds(), this->m_BarMoveSpeed);
		this->m_Scores[i] = score;

		float barHeight = goalHeight * score / gamemode->GetWinScore();
		x = origin.X + extent.X / 2.0f - graphWidth / 2.0f + axisSize + barGap / 2.0f + barSize * i + barGap * i;
		hud->DrawRect(color, x, y, barSize, -barHeight);

		this->DrawDashedRect(hud, color, gx, y - barHeight, graphWidth + axisSize, FMath::Max(1.0f, goalLineHeight * 0.5f), goalLineGap * 0.5f);

		FString teamText = FString::Printf(TEXT("P%d"), i + 1);
		hud->GetTextSize(teamText, width, height, hud->GetFont(), 0.38f * scale);
		hud->DrawText(teamText, this->m_GoalColor, x + barSize / 2.0f - width / 2.0f, y + axisSize * 1.5f, hud->GetFont(), 0.38f * scale);
	}
}