// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "HUD/HUDComponent.h"
#include "Character/PlayerCharacter.h"
#include "Gamemode/BaseGameMode.h"
#include "Gamemode/GMCapturePoints.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"
#include "HUD/Components/EndScreenComponent.h"
#include "HUD/Components/ScoresOverlayComponent.h"
#include "HUD/Components/GameOverCounterComponent.h"

#include "Engine/Canvas.h"

#define FONT_LOCATION TEXT("Font'/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField'")

AGameHUD::AGameHUD()
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(FONT_LOCATION);
	if (Font.Succeeded())
	{
		this->m_Font = Font.Object;
	}
	this->m_ScaleMaster = 1.0f;
	this->m_ScaleVertical = 0.003f;
	this->m_ScaleHorizontal = 0.005f;
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	this->m_PlayerCount = GetGameMode<ABaseGameMode>(Super::GetWorld())->GetPlayerCount();
	this->m_ControllerId = UGameplayStatics::GetPlayerControllerID(Super::GetOwningPlayerController());

	for (TSubclassOf<UHUDComponent> next : this->m_ComponentClasses)
	{
		if (next == nullptr)
		{
			continue;
		}
		this->m_Components.Add(NewObject<UHUDComponent>(this, next));
	}
}

void AGameHUD::ResetGameOver()
{
	UScoresOverlayComponent *component = this->FindComponent<UScoresOverlayComponent>();
	if (component != nullptr)
	{
		component->SetTitleText(TEXT(""));
	}
	UGameOverCounterComponent *gameover = this->FindComponent<UGameOverCounterComponent>();
	if (gameover != nullptr)
	{
		gameover->Reset();
	}
	//UEndScreenComponent *component = this->FindComponent<UEndScreenComponent>();
	//if (component != nullptr)
	//{
	//	//component->m_bRender = false;
	//}
	this->m_bGameOver = false;
}

void AGameHUD::SetGameOver(const int& winningTeam)
{
	UScoresOverlayComponent *component = this->FindComponent<UScoresOverlayComponent>();
	if (component != nullptr)
	{
		FString text = winningTeam == -1 ? TEXT("Times up!") : winningTeam == this->GetCharacter()->GetTeam()
			? TEXT("You Won!") : FString::Printf(TEXT("Player %d Won!"), winningTeam);
		component->SetTitleText(text);
	}
	//UEndScreenComponent *component = this->FindComponent<UEndScreenComponent>();
	//if(component != nullptr)
	//{
	//	//component->m_bRender = true;
	//}
	this->m_bGameOver = true;
}

void AGameHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Super::bShowHUD || this->GetCharacter() == nullptr)
	{
		return;
	}
	FVector2D origin(Super::Canvas->OrgX, Super::Canvas->OrgY), extent(Super::Canvas->SizeX, Super::Canvas->SizeY);
	FVector4 screen = FVector4(Super::Canvas->OrgX, Super::Canvas->OrgY, Super::Canvas->SizeX, Super::Canvas->SizeY);

	float scale = FMath::Min(screen.Z, screen.W);
	scale *= this->m_ScaleMaster;
	scale *= this->IsViewportVertical() ? this->m_ScaleVertical : this->m_ScaleHorizontal;

	for (UHUDComponent *component : this->m_Components)
	{
		if (this->m_bGameOver)
		{
			if (!component->IsA(UScoresOverlayComponent::StaticClass())
				&& !component->IsA(UGameOverCounterComponent::StaticClass())
				/*&& !component->IsA(UCurrentPlaceComponent::StaticClass())*/)
			{
				continue;
			}
			component->DrawComponent(this, origin, extent, scale, true);
		}
		else
		{
			if (this->m_bPaused && !component->m_bRenderWhenPaused)
			{
				continue;
			}
			component->DrawComponent(this, origin, extent, scale);
		}
	}
	if(this->m_bGameOver)
	{
		UGameOverCounterComponent *component = this->FindComponent<UGameOverCounterComponent>();
		this->m_bGameOverReady = component != nullptr && (component->IsReady() || this->GetControllerId() != this->GetPlayerCount() - 1);
	}
	//if (this->m_bGameOver)
	//{
	//	FString text = TEXT("Hold A to continue");

	//	APlayerController *controller = (APlayerController*)this->GetCharacter()->GetController();
	//	this->m_bGameOverReady = controller->IsInputKeyDown(EKeys::A) || controller->IsInputKeyDown(EKeys::Gamepad_FaceButton_Bottom);
	//	if (this->m_bGameOverReady)
	//	{
	//		text = TEXT("Ready");
	//	}
	//	float width, height;
	//	Super::GetTextSize(text, width, height, this->m_Font, scale * 0.5f);
	//	Super::DrawText(text, FLinearColor(0.9f, 0.9f, 0.9f, 0.75f), origin.X + extent.X / 2.0f - width / 2.0f, 
	//		origin.Y + extent.Y - height - 15 * scale, this->m_Font, scale * 0.5f);
	//}

	//if (this->GetCharacter()->GetHealth() <= 0.0f)
	//{
	//	Super::DrawText(TEXT("DEAD"), FLinearColor::Red, 100, 100, this->m_Font, scale * 2.0f);
	//}
}
