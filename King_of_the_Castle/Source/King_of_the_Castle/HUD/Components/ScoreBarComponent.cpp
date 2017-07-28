// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "ScoreBarComponent.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints.h"

#define IP_TIME_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)
#define IP_TIME_BOX_COLOR FLinearColor(0.05f, 0.05f, 0.05f, 0.8f)
#define IP_BAR_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)

UScoreBarComponent::UScoreBarComponent()
{
	this->m_TimeTextScale = 0.5f;
	this->m_TimeTextColor = IP_TIME_TEXT_COLOR;
	this->m_TimeBoxScale = 1.25f;
	this->m_TimeBoxColor = IP_TIME_BOX_COLOR;
	this->m_bRenderTime = true;

	this->m_BarWidth = 250.0f;
	this->m_BarHeight = 20.0f;
	this->m_BarAlpha = 0.5f;
	this->m_BarMoveSpeed = 1.0f;
	this->m_BarTextScale = 0.6f;
	this->m_BarTextColor = IP_BAR_TEXT_COLOR;
}

void UScoreBarComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	if (hud->GetControllerId() != hud->GetPlayerCount() - 1)
	{
		return;
	}
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(hud->GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	const int& playerCount = hud->GetPlayerCount(), teamCount = gamemode->GetTeamCount();

	float width, height;
	//float x = (playerCount == 2 || playerCount == 4) ? extent.X : extent.X / 2.0f,
	//	y = playerCount <= 2 ? 0.0f : extent.Y;
	float x = playerCount == 1 ? origin.X + extent.X / 2.0f : 0.0f;
	float y = origin.Y;

	//float barScale = this->m_BarWidth * scale;
	//for (int i = 0; i < gamemode->GetTeamCount(); i++)
	//{
	//	FLinearColor color = gamemode->GetTeamColor(i + 1);
	//	color.A = this->m_BarAlpha;

	//	float h = this->m_BarMinHeight * scale + (this->m_BarMaxHeight - this->m_BarMinHeight)
	//		* scale * gamemode->GetScore(i + 1) / gamemode->GetWinScore();
	//	hud->DrawRect(color, x - h / 2.0f, y + barScale * i - barScale * gamemode->GetTeamCount() / 2.0f, h, barScale);
	//}

	const float barWidth = this->m_BarWidth * scale, barHeight = this->m_BarHeight * scale;
	if (playerCount <= 2)
	{
		y += barHeight;
	}

	if (this->m_bRenderTime)
	{
		float timeScale = scale * this->m_TimeTextScale;
		int time = int(gamemode->GetGameDuration() - gamemode->GetTime());

		FString timeText = FString::Printf(TEXT("%02d:%02d"), time / 60, time % 60);
		hud->GetTextSize(timeText, width, height, hud->GetFont(), timeScale);

		if (playerCount <= 2)
		{
			y += height / 2.0f;
		}

		hud->DrawRect(this->m_TimeBoxColor, x - this->m_TimeBoxScale * width / 2.0f, y - this->m_TimeBoxScale
			* height - barHeight / 2.0f, this->m_TimeBoxScale * width, this->m_TimeBoxScale * height);
		hud->DrawText(timeText, this->m_TimeTextColor, x - width / 2.0f, y - height - barHeight / 2.0f, hud->GetFont(), timeScale);
	}

	// Calculate ratios
	TArray<float> ratios;
	float ratioTotal = 0.0f;

	if (this->m_TeamBarScores.Num() != teamCount)
	{
		this->m_TeamBarScores.Empty();
		this->m_TeamBarScores.Init(0.0f, teamCount);
	}

	for (int i = 0; i < teamCount; i++)
	{
		float score = FMath::FInterpConstantTo(this->m_TeamBarScores[i], gamemode->GetScore(i + 1),
			hud->GetWorld()->GetDeltaSeconds(), this->m_BarMoveSpeed);
		this->m_TeamBarScores[i] = score;

		float ratio = score / gamemode->GetWinScore();
		ratios.Add(ratio);
		ratioTotal += ratio;
	}
	if (ratioTotal == 0.0f)
	{
		ratios.Empty();
		ratios.Init(1.0f, teamCount);
		ratioTotal = teamCount;
	}

	// Render rect using normalize ratio
	float barX = x - barWidth / 2.0f;
	for (int i = 0; i < teamCount; i++)
	{
		FLinearColor color = gamemode->GetTeamColor(i + 1);
		color.A = this->m_BarAlpha;

		float w = barWidth * ratios[i] / ratioTotal;
		hud->DrawRect(color, barX, y - barHeight / 2.0f, w, barHeight);

		color *= 0.25f;
		color.A = 0.25f;
		//Super::DrawRect(color, barX, y - barHeight / 2.0f, w * 0.1f, barHeight);
		//Super::DrawRect(color, barX + w, y - barHeight / 2.0f, -w * 0.1f, barHeight);
		hud->DrawRect(color, barX, y - barHeight / 2.0f, w, barHeight * 0.15f);
		hud->DrawRect(color, barX, y + barHeight / 2.0f, w, -barHeight * 0.15f);

		FString score = FString::Printf(TEXT("%d"), gamemode->GetScore(i + 1));
		hud->GetTextSize(score, width, height, hud->GetFont(), this->m_BarTextScale * scale);

		if (w > width)
		{
			hud->DrawText(score, this->m_BarTextColor, barX + w / 2.0f - width / 2.0f, y - height / 2.0f, hud->GetFont(), this->m_BarTextScale * scale);
		}
		barX += w;
	}
}