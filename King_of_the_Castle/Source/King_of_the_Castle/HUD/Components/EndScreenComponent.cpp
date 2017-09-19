// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "EndScreenComponent.h"

#include "Gamemode/GMCapturePoints.h"

#define CONTAINER_BORDER_COLOR FLinearColor(0.97f, 0.42f, 0.22f, 0.8f)
#define CONTAINER_BACKGROUND_COLOR FLinearColor(0.0f, 0.64f, 0.9f, 0.7f)

#define TITLE_TEXT_COLOR FLinearColor(0.98f, 0.98f, 0.98f, 0.9f)

#define END_BACKGROUND_COLOR FLinearColor(0.0015f, 0.0020f, 0.0025f, 0.8f)

#define GET_COLOR(color, aMultiplier) FLinearColor(color.R, color.G, color.B, color.A * aMultiplier)

UEndScreenComponent::UEndScreenComponent()
{
	this->m_Counter = 0.0f;
	this->m_WaitTimer = 7.0f;
	this->m_ContainerWidth = 0.5f;
	this->m_ContainerHeight = 0.6f;
	this->m_ContainerWaitDelay = 0.1f;
	this->m_ContainerFadeTimer = 1.0f;
	this->m_ContainerBorderSize = 6.0f;
	this->m_ContainerBkgColor = CONTAINER_BACKGROUND_COLOR;
	this->m_ContainerBorderColor = CONTAINER_BORDER_COLOR;
	this->m_TitleTextY = -6.0f;
	this->m_TitleTextSize = 2.0f;
	this->m_TitleText = TEXT("The End");
	this->m_TitleTextColor = TITLE_TEXT_COLOR;
	this->m_PlaceSize = 30.0f;
	this->m_BkgFadeTimer = 2.25f;
	this->m_BkgColor = END_BACKGROUND_COLOR;

	Super::m_bRender = false;
	Super::m_bRenderOnLast = true;
}

void UEndScreenComponent::Render(AGameHUD* hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(hud->GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	this->m_Counter += hud->GetWorld()->GetDeltaSeconds();

	FLinearColor bkgColor = this->m_BkgColor;
	if (this->m_Counter < this->m_BkgFadeTimer)
	{
		bkgColor.A *= this->m_Counter / this->m_BkgFadeTimer;
	}

	// Background
	float x = origin.X - extent.X, y = origin.Y - extent.Y, width = extent.X * 2.0f, height = extent.Y * 2.0f;
	hud->DrawRect(bkgColor, x, y, width, height);

	// Set to container
	width = width * this->m_ContainerWidth;
	height = height * this->m_ContainerHeight;
	x = origin.X - width / 2.0f;
	y = origin.Y - height / 2.0f;

	if (this->m_Counter < this->m_ContainerWaitDelay)
	{
		return;
	}
	float alphaMulti = (this->m_Counter - this->m_ContainerWaitDelay) >= this->m_ContainerFadeTimer ? 1.0f
		: FMath::Sin(HALF_PI * (this->m_Counter - this->m_ContainerWaitDelay) / this->m_ContainerFadeTimer);

	FLinearColor conBkgColor = GET_COLOR(this->m_ContainerBkgColor, alphaMulti);
	FLinearColor conBorderColor = GET_COLOR(this->m_ContainerBorderColor, alphaMulti);
	Super::DrawBorderBox(hud, conBkgColor, x, y, width, height, this->m_ContainerBorderSize * scale, conBorderColor);

	// Title
	float titleScale = this->m_TitleTextSize * scale, titleWidth, titleHeight;
	FLinearColor titleColor = GET_COLOR(this->m_TitleTextColor, alphaMulti);
	hud->GetTextSize(this->m_TitleText, titleWidth, titleHeight, hud->GetFont(), titleScale);
	hud->DrawText(this->m_TitleText, titleColor, x + width / 2.0f - titleWidth / 2.0f,
		y + titleHeight / 4.0f + this->m_TitleTextY * scale, hud->GetFont(), titleScale);

	float titleLineSize = 4.0f * scale;
	hud->DrawRect(titleColor, x + width / 2.0f - titleWidth / 2.0f - titleLineSize, y + titleHeight / 4.0f
		+ this->m_TitleTextY * scale + titleHeight - titleLineSize, titleWidth + titleLineSize * 2.0f, titleLineSize);

	// Separator
	FLinearColor sepColor = conBkgColor * 0.15f;
	sepColor.A = conBkgColor.A;
	hud->DrawRect(sepColor, x + width * 0.3f, y + height * 0.3f, 8.0f * scale, height * 0.6f);

	// Render place
	float placeHeight = height * 0.5f, placeY = y + height * 0.3f + (height * 0.6f - placeHeight) / 2.0f;
	float placeTextScale = 1.0f * scale;
	//TMap<int, int> placeOrder;
	for(int i = 0; i < 4; i++)
	{
		//placeOrder[i] = gamemode->GetPlace(i + 1);
	}
	//placeOrder.ValueSort([](int a, int b) { return a < b; });
	for (int i = 0; i < gamemode->GetTeamCount(); i++)
	{
		//gamemode->GetPlace()
		float xOffset = width * 0.3f, yOffset = i * placeHeight / gamemode->GetTeamCount();
		float pTextWidth, pTextHeight, nTextWidth, nTextHeight;
		FString pText = FString::Printf(TEXT("%d%s"), i + 1, i == 0 ? TEXT("st") 
			: i == 1 ? TEXT("nd") : i == 2 ? TEXT("rd") : TEXT("th"));
		FString nText = FString::Printf(TEXT("Player %d"), gamemode->GetPlace(i + 1));

		hud->GetTextSize(pText, pTextWidth, pTextHeight, hud->GetFont(), placeTextScale);
		hud->GetTextSize(nText, nTextWidth, nTextHeight, hud->GetFont(), placeTextScale * 0.9f);

		//hud->DrawText(pText, FLinearColor::White, x + xOffset - pTextWidth - nTextWidth, placeY + yOffset + pTextHeight / 2.0f, hud->GetFont(), placeTextScale);
		hud->DrawText(nText, FLinearColor::White, x + xOffset - nTextWidth, placeY + yOffset + nTextHeight / 2.0f, hud->GetFont(), placeTextScale * 0.9f);
		//hud->DrawTexture(this->m_PlaceTextures[i], x + xOffset, placeY + yOffset, placeSize, placeSize, 0.0f, 0.0f, 1.0f, 1.0f);
	}
	//hud->DrawRect(FLinearColor::Red, x + width * 0.3f, placeY, placeSize, placeHeight);
}
