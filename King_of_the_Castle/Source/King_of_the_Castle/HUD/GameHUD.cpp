// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameHUD.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/BaseGameMode.h"
#include "Gamemode/GMCapturePoints.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#include "Engine/Canvas.h"

#define CROSSHAIR_SIZE 5.0f // Multiplier of min(width, height)
#define CROSSHAIR_COLOR FLinearColor(0.25f, 0.75f, 0.75f, 0.5f)

#define HEALTH_FULL_COLOR FLinearColor(0.1f, 0.8f, 0.1f, 0.75f)
#define HEALTH_EMPTY_COLOR FLinearColor(1.0f, 0.1f, 0.1f, 0.75f)
#define STAMINA_COLOR FLinearColor(0.1f, 0.2f, 0.9f, 0.75f)

#define CP_OWNED_COLOR FLinearColor(0.0f, 0.75f, 0.0f, 0.25f)
#define CP_UNOWNED_COLOR  FLinearColor(0.8f, 0.0f, 0.0f, 0.25f)
#define CP_TEXT_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 0.8f)

#define IP_TIME_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)

#define PLACE_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)

#define CP_MATERIAL_PARAM_ALPHA TEXT("Alpha")
#define CP_MATERIAL_PARAM_BOX_COLOR TEXT("Color")
#define CP_MATERIAL_PARAM_TEAM_COLOR TEXT("TeamColor")
#define CP_MATERIAL_PARAM_TEAM_SIZE TEXT("TeamBoxSize")
#define CP_MATERIAL_PARAM_CAPTURE_PERC TEXT("TeamCapturePerc")
#define CP_MATERIAL_PARAM_CAPTURE_COLOR TEXT("TeamCaptureColor")
#define CP_MATERIAL_PARAM_FLASH_SPEED TEXT("BoxFlashSpeed")

#define FONT_LOCATION TEXT("Font'/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField'")

AGameHUD::AGameHUD() : m_bCrosshairVisible(false), m_bScoresTableVisible(false)
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(FONT_LOCATION);
	if (Font.Succeeded())
	{
		this->m_Font = Font.Object;
	}

	// Scale
	this->m_ScaleMaster = 1.0f;
	this->m_ScaleVertical = 0.003f;
	this->m_ScaleHorizontal = 0.005f;

	// Health and Stamina bars
	this->m_BarHealthMaxColor = HEALTH_FULL_COLOR;
	this->m_BarHealthMinColor = HEALTH_EMPTY_COLOR;
	this->m_BarStaminaColor = STAMINA_COLOR;
	this->m_BarHealthText = TEXT("HP");
	this->m_BarStaminaText = TEXT("SP");
	this->m_BarTextScale = 0.4f;
	this->m_BarScaleXHealth = 120.0f;
	this->m_BarScaleXStamina = 90.0f;
	this->m_BarScaleY = 10.0f;
	this->m_BarSeparation = 2.5f;
	this->m_BarPadding = 12.0f;
	this->m_BarMoveSpeed = 2.0f;
	this->m_bRenderBars = true;

	// Capture Points
	this->m_CPOwnedColor = CP_OWNED_COLOR;
	this->m_CPUnownedColor = CP_UNOWNED_COLOR;
	this->m_CPBoxScale = 31.0f;
	this->m_CPTeamBoxUVSize = 0.12f;
	this->m_CPBoxAlpha = 0.5f;
	this->m_CPBoxFlashSpeed = 0.65f;
	this->m_CPTextColor = CP_TEXT_COLOR;
	this->m_CPTextScale = 0.58f;
	this->m_CPTextZOffset = 140.0f;
	this->m_bRenderCapturePoints = true;

	// Info Panel
	this->m_IPTimeBoxScale = 1.25f;
	this->m_IPTimeTextScale = 0.5f;
	this->m_IPTimeTextColor = IP_TIME_TEXT_COLOR;
	this->m_bRenderTime = true;
	this->m_IPTeamBarWidth = 250.0f;
	this->m_IPTeamBarHeight = 20.0f;
	this->m_IPTeamBarAlpha = 0.5f;
	this->m_IPTeamBarMoveSpeed = 1.0f;
	this->m_bRenderInfoPanel = true;

	// Score Place
	this->m_PlaceTextScale = 1.5f;
	this->m_PlaceSubTextScale = 0.5f;
	this->m_PlacePadding = 12.0f;
	this->m_PlaceTextColor = PLACE_TEXT_COLOR;
	this->m_bRenderScorePlace = true;
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	this->m_PlayerCount = GetGameMode<ABaseGameMode>(Super::GetWorld())->GetPlayerCount();
	this->m_ControllerId = UGameplayStatics::GetPlayerControllerID(Super::GetOwningPlayerController());
}

void AGameHUD::RenderPlace(const FVector4& screen, const float& scale)
{
	ABaseGameMode *gamemode = GetGameMode<ABaseGameMode>(Super::GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	int32 place = gamemode->GetPlace(this->GetCharacter()->GetTeam());
	FString placeStr = FString::FromInt(place), subStr = place == 1 ? TEXT("st")
		: place == 2 ? TEXT("nd") : place == 3 ? TEXT("rd") : TEXT("th");

	float numWidth, numHeight, subWidth, subHeight;
	Super::GetTextSize(placeStr, numWidth, numHeight, this->m_Font, scale * this->m_PlaceTextScale);
	Super::GetTextSize(subStr, subWidth, subHeight, this->m_Font, scale * this->m_PlaceSubTextScale);

	float padding = scale * this->m_PlacePadding;
	float x = (this->IsOpposite() ? (screen.X + screen.Z - numWidth - padding) : screen.X + padding) - subWidth / 2.0f;
	float y = screen.W - numHeight - padding / 2.0f;

	Super::DrawText(placeStr, this->m_PlaceTextColor, x, y, this->m_Font, scale * this->m_PlaceTextScale);

	Super::DrawText(subStr, this->m_PlaceTextColor, x + numWidth, y, this->m_Font, scale * this->m_PlaceSubTextScale);
}

void AGameHUD::RenderBars(const FVector4& screen, const float& scale)
{
	APlayerCharacter *character = this->GetCharacter();

	float padding = scale * this->m_BarPadding;
	float x = screen.X + padding, y = screen.Y + padding / 2.0f;

	float widthHealth = scale * this->m_BarScaleXHealth, widthStamina = scale * this->m_BarScaleXStamina;
	float height = scale * this->m_BarScaleY;

	// If the bars should be displayed on the opposite side (x)
	if (this->IsOpposite())
	{
		x = screen.X + screen.Z - padding;
		widthHealth = -widthHealth;
		widthStamina = -widthStamina;
	}

	float textWidth, textHeight, offset;
	// Health
	float healthPerc = character->GetHealth() / character->GetMaxHealth();
	healthPerc = this->m_HealthPerc = FMath::FInterpTo(this->m_HealthPerc, healthPerc,
		Super::GetWorld()->GetDeltaSeconds(), this->m_BarMoveSpeed);

	FLinearColor healthColor = this->m_BarHealthMinColor + (m_BarHealthMaxColor - m_BarHealthMinColor) * healthPerc;

	Super::GetTextSize(this->m_BarHealthText, textWidth, textHeight, this->m_Font, scale * this->m_BarTextScale);
	Super::DrawText(this->m_BarHealthText, FLinearColor::White, x - textWidth / 2.0f,
		y + height / 2.0f - textHeight / 2.0f, this->m_Font, scale * this->m_BarTextScale);

	offset = this->IsOpposite() ? -(textWidth / 4.0f + padding / 2.0f) : (textWidth / 4.0f + padding / 2.0f);
	Super::DrawRect(healthColor, x + offset, y, widthHealth * healthPerc, height * 0.6f);
	Super::DrawRect(healthColor * 0.8f, x + offset, y + height * 0.6f, widthHealth * healthPerc, height * 0.4f);

	Super::DrawRect(healthColor * 0.4f, x + offset + widthHealth * healthPerc, y, widthHealth * (1.0f - healthPerc), height);

	// Stamina
	float staminaPerc = character->GetStamina() / character->GetMaxStamina();
	staminaPerc = this->m_StaminaPerc = FMath::FInterpTo(this->m_StaminaPerc, staminaPerc,
		Super::GetWorld()->GetDeltaSeconds(), this->m_BarMoveSpeed);

	y += height + scale * this->m_BarSeparation;

	Super::GetTextSize(this->m_BarHealthText, textWidth, textHeight, this->m_Font, scale * this->m_BarTextScale);
	Super::DrawText(this->m_BarStaminaText, FLinearColor::White, x - textWidth / 2.0f,
		y + height / 2.0f - textHeight / 2.0f, this->m_Font, scale * this->m_BarTextScale);

	offset = this->IsOpposite() ? -(textWidth / 4.0f + padding / 2.0f) : (textWidth / 4.0f + padding / 2.0f);
	Super::DrawRect(this->m_BarStaminaColor * 0.8f, x + offset, y, widthStamina * staminaPerc, height * 0.4f);
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
		Super::GetTextSize(name, width, height, this->m_Font, textScale);
		Super::DrawText(name, this->m_CPTextColor, position.X - width / 2.0f,
			position.Y - height / 2.0f, this->m_Font, textScale);
	}
}

void AGameHUD::RenderScores(const FVector4& screen, const float& scale)
{
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	float x, y;
	float width, height;

	// Render gamemode name
	FString name = gamemode->GetGameModeName().ToString();
	Super::GetTextSize(name, width, height, this->m_Font, scale * 0.8f);

	x = screen.X + screen.Z / 2.0f - width / 2.0f;
	y = screen.Y + screen.W * 0.15f - height / 2.0f;

	float offset = scale * 8.0f;
	Super::DrawRect(FLinearColor::Blue, x - offset / 2.0f, y - offset / 4.0f, width + offset, height + offset / 2.0f);
	Super::DrawText(name, FLinearColor::White, x, y, this->m_Font, scale * 0.8f);

	// Render graph axis
	float graphWidth = scale * 180.0f, graphHeight = scale * 100.0f;

	float axisSize = scale * 3.0f, axisExtra = scale * 10.0f;
	// x axis
	Super::DrawRect(FLinearColor::Red, screen.X + screen.Z / 2.0f - graphWidth / 2.0f - axisExtra,
		screen.Y + screen.W / 2.0f + graphHeight / 2.0f, graphWidth + axisExtra * 2.0f, axisSize);
	// y axis
	Super::DrawRect(FLinearColor::Red, screen.X + screen.Z / 2.0f - graphWidth / 2.0f,
		screen.Y + screen.W / 2.0f - graphHeight / 2.0f, axisSize, graphHeight + axisExtra);

	float barGoalHeight = scale * 20.0f;
	float barGap = scale * 20.0f, barSize = (graphWidth - barGap - barGap * (gamemode->GetTeamCount() - 1)) / gamemode->GetTeamCount();
	for (int i = 0; i < gamemode->GetTeamCount(); i++)
	{
		float barHeight = (graphHeight - barGoalHeight) * 1.0f;// (gamemode->GetScore(i + 1) / gamemode->GetWinScore());
		Super::DrawRect(gamemode->GetTeamColor(i + 1),
			screen.X + screen.Z / 2.0f - graphWidth / 2.0f + axisSize + barGap / 2.0f + barSize * i + barGap * i,
			screen.Y + screen.W / 2.0f + graphHeight / 2.0f, barSize, -barHeight);
	}

	// Draw goal line
	float goalLineHeight = scale * 2.0f;
	Super::DrawRect(FLinearColor::Green, screen.X + screen.Z / 2.0f - graphWidth / 2.0f, 
		screen.Y + screen.W / 2.0f - graphHeight / 2.0f + barGoalHeight - goalLineHeight, graphWidth, goalLineHeight);
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

	//float barScale = this->m_IPTeamBarWidth * scale;
	//for (int i = 0; i < gamemode->GetTeamCount(); i++)
	//{
	//	FLinearColor color = gamemode->GetTeamColor(i + 1);
	//	color.A = this->m_IPTeamBarAlpha;

	//	float h = this->m_IPTeamBarMinHeight * scale + (this->m_IPTeamBarMaxHeight - this->m_IPTeamBarMinHeight)
	//		* scale * gamemode->GetScore(i + 1) / gamemode->GetWinScore();
	//	Super::DrawRect(color, x - h / 2.0f, y + barScale * i - barScale * gamemode->GetTeamCount() / 2.0f, h, barScale);
	//}

	const float barWidth = this->m_IPTeamBarWidth * scale, barHeight = this->m_IPTeamBarHeight * scale;
	if (this->m_PlayerCount <= 2)
	{
		y += barHeight;
	}

	if (this->m_bRenderTime)
	{
		float timeScale = scale * this->m_IPTimeTextScale;
		int time = int(gamemode->GetGameDuration() - gamemode->GetTime());

		FString timeText = FString::Printf(TEXT("%02d:%02d"), time / 60, time % 60);
		Super::GetTextSize(timeText, width, height, this->m_Font, timeScale);

		if (this->m_PlayerCount <= 2)
		{
			y += height / 2.0f;
		}

		Super::DrawRect(FLinearColor(0.05f, 0.05f, 0.05f, 0.8f), x - this->m_IPTimeBoxScale * width / 2.0f, y - this->m_IPTimeBoxScale
			* height - barHeight / 2.0f, this->m_IPTimeBoxScale * width, this->m_IPTimeBoxScale * height);
		Super::DrawText(timeText, this->m_IPTimeTextColor, x - width / 2.0f, y - height - barHeight / 2.0f, this->m_Font, timeScale);
	}

	TArray<float> ratios;
	float ratioTotal = 0.0f;

	if (this->m_TeamBarScores.Num() != gamemode->GetTeamCount())
	{
		this->m_TeamBarScores.Empty();
		this->m_TeamBarScores.Init(0.0f, gamemode->GetTeamCount());
	}
	// Calculate ratios
	for (int i = 0; i < gamemode->GetTeamCount(); i++)
	{
		float score = FMath::FInterpConstantTo(this->m_TeamBarScores[i], gamemode->GetScore(i + 1),
			Super::GetWorld()->GetDeltaSeconds(), this->m_IPTeamBarMoveSpeed);
		this->m_TeamBarScores[i] = score;

		float ratio = score / gamemode->GetWinScore();
		ratios.Add(ratio);
		ratioTotal += ratio;
	}
	if (ratioTotal == 0.0f)
	{
		ratios.Empty();
		ratios.Init(1.0f, gamemode->GetTeamCount());
		ratioTotal = gamemode->GetTeamCount();
	}
	// Render rect using normalize ratio
	float barX = x - barWidth / 2.0f;
	for (int i = 0; i < gamemode->GetTeamCount(); i++)
	{
		FLinearColor color = gamemode->GetTeamColor(i + 1);
		color.A = this->m_IPTeamBarAlpha;

		float w = barWidth * ratios[i] / ratioTotal;
		Super::DrawRect(color, barX, y - barHeight / 2.0f, w, barHeight);

		color = FLinearColor(color.R * 0.25f, color.G * 0.25f, color.B * 0.25f, 0.25f);
		//Super::DrawRect(color, barX, y - barHeight / 2.0f, w * 0.1f, barHeight);
		//Super::DrawRect(color, barX + w, y - barHeight / 2.0f, -w * 0.1f, barHeight);
		Super::DrawRect(color, barX, y - barHeight / 2.0f, w, barHeight * 0.15f);
		Super::DrawRect(color, barX, y + barHeight / 2.0f, w, -barHeight * 0.15f);

		FString score = FString::Printf(TEXT("%d"), gamemode->GetScore(i + 1));
		Super::GetTextSize(score, width, height, this->m_Font, scale * 0.6f);

		if (w > width)
		{
			Super::DrawText(score, this->m_IPTimeTextColor, barX + w / 2.0f - width / 2.0f, y - height / 2.0f, this->m_Font, scale * 0.6f);
		}
		barX += w;
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
	if (this->m_bRenderScorePlace)
	{
		this->RenderPlace(screen, scale);
	}
	if (this->m_bRenderInfoPanel)
	{
		this->RenderForAll(screen, scale);
	}
	if (this->m_bCrosshairVisible)
	{
		const float& size = FMath::RoundToFloat(scale * CROSSHAIR_SIZE);
		Super::DrawRect(CROSSHAIR_COLOR, screen.X + screen.Z / 2.0f - size / 2.0f,
			screen.Y + screen.W / 2.0f - size / 2.0f, size, size);
	}
	//this->m_BuildWheel->Render(this, screen);

	if (this->GetCharacter()->GetHealth() <= 0.0f)
	{
		Super::DrawText(TEXT("DEAD"), FLinearColor::Red, 100, 100, this->m_Font, scale * 2.0f);
	}

	if (this->m_bScoresTableVisible)
	{
		this->RenderScores(screen, scale);
	}
}
