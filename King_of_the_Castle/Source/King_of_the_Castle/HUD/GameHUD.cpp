// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "Gamemode/BaseGameMode.h"
#include "Gamemode/GMCapturePoints.h"
#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#include "Engine/Canvas.h"

#define CROSSHAIR_SIZE 0.016f // Multiplier of min(width, height)
#define CROSSHAIR_COLOR FLinearColor(0.25f, 0.75f, 0.75f, 0.5f)

#define HEALTH_COLOR FLinearColor(1.0f, 0.1f, 0.1f, 0.75f)
#define STAMINA_COLOR FLinearColor(0.1f, 0.8f, 0.1f, 0.75f)

#define CP_OWNED_COLOR FLinearColor(0.0f, 0.75f, 0.0f, 0.25f)
#define CP_UNOWNED_COLOR  FLinearColor(0.8f, 0.0f, 0.0f, 0.25f)
#define CP_TEXT_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.8f)

#define IP_TIME_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)

#define CP_MATERIAL_PARAM_ALPHA TEXT("Alpha")
#define CP_MATERIAL_PARAM_BOX_COLOR TEXT("Color")
#define CP_MATERIAL_PARAM_TEAM_COLOR TEXT("TeamColor")
#define CP_MATERIAL_PARAM_TEAM_SIZE TEXT("TeamBoxSize")
#define CP_MATERIAL_PARAM_CAPTURE_PERC TEXT("TeamCapturePerc")
#define CP_MATERIAL_PARAM_CAPTURE_COLOR TEXT("TeamCaptureColor")
#define CP_MATERIAL_PARAM_FLASH_SPEED TEXT("BoxFlashSpeed")

AGameHUD::AGameHUD() : m_bCrosshairVisible(false)
{
	// Scale
	this->m_ScaleMaster = 1.0f;
	this->m_ScaleVertical = 0.003f;
	this->m_ScaleHorizontal = 0.005f;

	// Health and Stamina bars
	this->m_BarHealthColor = HEALTH_COLOR;
	this->m_BarStaminaColor = STAMINA_COLOR;
	this->m_BarHealthText = TEXT("HP");
	this->m_BarStaminaText = TEXT("SP");
	this->m_BarTextScale = 0.65f;
	this->m_BarScaleXHealth = 120.0f;
	this->m_BarScaleXStamina = 90.0f;
	this->m_BarScaleY = 10.0f;
	this->m_BarSeparation = 2.5f;
	this->m_BarPadding = 12.0f;
	this->m_bRenderBars = true;

	// Capture Points
	this->m_CPOwnedColor = CP_OWNED_COLOR;
	this->m_CPUnownedColor = CP_UNOWNED_COLOR;
	this->m_CPBoxScale = 18.0f;
	this->m_CPTeamBoxUVSize = 0.12f;
	this->m_CPBoxAlpha = 0.5f;
	this->m_CPBoxFlashSpeed = 0.65f;
	this->m_CPTextColor = CP_TEXT_COLOR;
	this->m_CPTextScale = 1.0f;
	this->m_CPTextZOffset = 140.0f;
	this->m_bRenderCapturePoints = true;

	// Info Panel
	this->m_IPTimeBoxScale = 1.25f;
	this->m_IPTimeTextScale = 1.1f;
	this->m_IPTimeTextColor = IP_TIME_TEXT_COLOR;
	this->m_bRenderTime = true;
	this->m_bRenderInfoPanel = true;

	//this->m_BuildWheel = UObject::CreateDefaultSubobject<UBuildWheel>(TEXT("BuildWheel"));
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	this->m_PlayerCount = GetGameMode<ABaseGameMode>(Super::GetWorld())->GetPlayerCount();
	this->m_ControllerId = UGameplayStatics::GetPlayerControllerID(Super::GetOwningPlayerController());
}

void AGameHUD::RenderBars(const FVector4& screen, const float& scale)
{
	APlayerCharacter *character = this->GetCharacter();

	float padding = scale * this->m_BarPadding;
	float x = screen.X + padding, y = screen.Y + padding / 2.0f;

	float widthHealth = scale * this->m_BarScaleXHealth, widthStamina = scale * this->m_BarScaleXStamina;
	float height = scale * this->m_BarScaleY;

	// If the bars should be displayed on the opposite side (x)
	bool opposite = (this->m_PlayerCount == 2 && this->m_ControllerId == 1)
		|| (this->m_PlayerCount == 3 && this->m_ControllerId == 2)
		|| (this->m_PlayerCount == 4 && (this->m_ControllerId == 1 || this->m_ControllerId == 3));
	if (opposite)
	{
		x = screen.X + screen.Z - padding;
		widthHealth = -widthHealth;
		widthStamina = -widthStamina;
	}

	float textWidth, textHeight, offset;
	// Health
	float healthPerc = character->GetHealth() / character->GetMaxHealth();

	Super::GetTextSize(this->m_BarHealthText, textWidth, textHeight, nullptr, scale * this->m_BarTextScale);
	Super::DrawText(this->m_BarHealthText, FLinearColor::White, x - textWidth / 2.0f,
		y + height / 2.0f - textHeight / 2.0f, nullptr, scale * this->m_BarTextScale);

	offset = opposite ? -(textWidth / 4.0f + padding / 2.0f) : (textWidth / 4.0f + padding / 2.0f);
	Super::DrawRect(this->m_BarHealthColor, x + offset, y, widthHealth * healthPerc, height * 0.6f);
	Super::DrawRect(this->m_BarHealthColor * 0.9f, x + offset, y + height * 0.6f, widthHealth * healthPerc, height * 0.4f);

	Super::DrawRect(this->m_BarHealthColor * 0.4f, x + offset + widthHealth * healthPerc, y, widthHealth * (1.0f - healthPerc), height);

	// Stamina
	float staminaPerc = character->GetStamina() / character->GetMaxStamina();

	y += height + scale * this->m_BarSeparation;

	Super::GetTextSize(this->m_BarHealthText, textWidth, textHeight, nullptr, scale * this->m_BarTextScale);
	Super::DrawText(this->m_BarStaminaText, FLinearColor::White, x - textWidth / 2.0f,
		y + height / 2.0f - textHeight / 2.0f, nullptr, scale * this->m_BarTextScale);

	offset = opposite ? -(textWidth / 4.0f + padding / 2.0f) : (textWidth / 4.0f + padding / 2.0f);
	Super::DrawRect(this->m_BarStaminaColor * 0.9f, x + offset, y, widthStamina * staminaPerc, height * 0.4f);
	Super::DrawRect(this->m_BarStaminaColor, x + offset, y + height * 0.4f, widthStamina * staminaPerc, height * 0.6f);

	Super::DrawRect(this->m_BarStaminaColor * 0.4f, x + offset + widthStamina * staminaPerc, y, widthStamina * (1.0f - staminaPerc), height);
}

void AGameHUD::RenderCapturePoints(const FVector4& screen, const float& scale)
{
	if (this->m_CPMaterial == nullptr)
	{
		return;
	}
	ABaseGameMode *gamemode = GetGameMode<ABaseGameMode>(Super::GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	const int& team = this->GetCharacter()->GetTeam();

	TArray<AActor*> points;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ACapturePoint::StaticClass(), points);

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
		FVector position = Super::Project(points[i]->GetActorLocation() + FVector(0.0f, 0.0f, this->m_CPTextZOffset));

		float textScale = scale * this->m_CPTextScale, boxScale = textScale * this->m_CPBoxScale;
		if (position.Z <= 0.0f 
			|| position.X < boxScale / 2.0f || position.X > screen.Z - boxScale / 2.0f
			|| position.Y < boxScale / 2.0f || position.Y > screen.W - boxScale / 2.0f)
		{
			continue;
		}
		// Draw background box
		if (!this->m_CPMaterials.Contains(point))
		{
			this->m_CPMaterials.Add(point, UMaterialInstanceDynamic::Create(this->m_CPMaterial, Super::GetOuter()));
		}
		UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_CPMaterials[point]);
		if (material == nullptr)
		{
			continue; 
		}
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_ALPHA, this->m_CPBoxAlpha);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_FLASH_SPEED, this->m_CPBoxFlashSpeed);
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_BOX_COLOR, team == point->GetOwningTeam() ? this->m_CPOwnedColor : this->m_CPUnownedColor);
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_TEAM_COLOR, gamemode->GetTeamColor(point->GetOwningTeam()));
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_TEAM_SIZE, this->m_CPTeamBoxUVSize);
		material->SetScalarParameterValue(CP_MATERIAL_PARAM_CAPTURE_PERC, point->GetCapturePercentage());
		material->SetVectorParameterValue(CP_MATERIAL_PARAM_CAPTURE_COLOR, gamemode->GetTeamColor(point->GetCapturingTeam()));

		//position.X = FMath::Max(boxScale, FMath::Min(screen.Z - boxScale, position.X));
		//position.Y = FMath::Max(boxScale, FMath::Min(screen.W - boxScale, position.Y));

		Super::DrawMaterialSimple(material, position.X - boxScale / 2.0f, position.Y - boxScale / 2.0f, boxScale, boxScale);

		// Draw Text
		float width, height;
		Super::GetTextSize(name, width, height, nullptr, textScale);
		Super::DrawText(name, this->m_CPTextColor, position.X - width / 2.0f,
			position.Y - height / 2.0f, nullptr, textScale);
	}
}

void AGameHUD::RenderForAll(const FVector4& screen, const float& scale)
{
	if (this->m_ControllerId != 0)
	{
		return;
	}
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	float width, height;
	float x = (this->m_PlayerCount == 2 || this->m_PlayerCount == 4) ? screen.Z : screen.Z / 2.0f,
		y = (this->m_PlayerCount <= 2) ? 0.0f : screen.W;

	if (this->m_bRenderTime)
	{
		float timeScale = scale * this->m_IPTimeTextScale;
		int time = int(gamemode->GetGameDuration() - gamemode->GetTime());

		FString timeText = FString::Printf(TEXT("%02d:%02d"), time / 60, time % 60);
		Super::GetTextSize(timeText, width, height, nullptr, timeScale);

		Super::DrawRect(FLinearColor::Blue, x - this->m_IPTimeBoxScale * width / 2.0f, y - this->m_IPTimeBoxScale
			* height / 2.0f, this->m_IPTimeBoxScale * width, this->m_IPTimeBoxScale * height);
		Super::DrawText(timeText, this->m_IPTimeTextColor, x - width / 2.0f, y - height / 2.0f, nullptr, timeScale);
	}
}

void AGameHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Super::bShowHUD)
	{
		return;
	}
	FVector4 screen = FVector4(Super::Canvas->OrgX, Super::Canvas->OrgY, Super::Canvas->SizeX, Super::Canvas->SizeY);

	float scale = FMath::Min(screen.Z, screen.W);
	scale *= this->m_ScaleMaster;
	scale *= this->IsViewportVertical() ? this->m_ScaleVertical : this->m_ScaleHorizontal;

	this->RenderCapturePoints(screen, scale);

	// Render health and stamina bars
	if (this->m_bRenderBars)
	{
		this->RenderBars(screen, scale);
	}
	this->RenderForAll(screen, scale);
	if (this->m_bCrosshairVisible)
	{
		const float& size = FMath::RoundToFloat(scale * CROSSHAIR_SIZE);
		Super::DrawRect(CROSSHAIR_COLOR, screen.X + screen.Z / 2.0f - size / 2.0f,
			screen.Y + screen.W / 2.0f - size / 2.0f, size, size);
	}
	//this->m_BuildWheel->Render(this, screen);

	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
	if (gamemode != nullptr)
	{
		Super::DrawText(FString::Printf(TEXT("Score: %d"), gamemode->GetScore(this->GetCharacter()->GetTeam())),
			FColor::White, 10.0f, screen.W - 30.0f, nullptr, scale * 0.8f);
	}
}
