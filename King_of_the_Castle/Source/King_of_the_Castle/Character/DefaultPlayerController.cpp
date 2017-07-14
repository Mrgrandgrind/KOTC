// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "DefaultPlayerController.h"

#include "Gamemode/BaseGameMode.h"
#include "Construction/BlockData.h"
#include "Character/PlayerCharacter.h"
#include "Construction/Brush/PrimaryBrush.h"
#include "Construction/BlockStructureManager.h"

APlayerCharacter* ADefaultPlayerController::GetCharacter() const
{
	return Cast<APlayerCharacter>(Super::GetPawn());
}

void ADefaultPlayerController::KOTC_Respawn()
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->Stun(0.01f, false, true);
	}
}

void ADefaultPlayerController::KOTC_Stun(const float& duration)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->Stun(duration);
	}
}

void ADefaultPlayerController::KOTC_SetTeam(const int& team)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->SetTeam(team);
	}
}

void ADefaultPlayerController::KOTC_SetHealth(const float& health)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->SetHealth(health);
	}
}

void ADefaultPlayerController::KOTC_SetStamina(const float& stamina)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->SetStamina(stamina);
	}
}

void ADefaultPlayerController::KOTC_SetBlockReach(const float& reach)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->SetBuildReach(reach <= 0 ? DEFAULT_REACH_DISTANCE 
			: (reach * KOTC_CONSTRUCTION_REACH_MULTIPLIER));
	}
}

void ADefaultPlayerController::KOTC_SetBlockCount(const int& count)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character == nullptr)
	{
		return;
	}
	UPrimaryBrush *brush = character->GetPrimaryBrush();
	UBlockData *data = brush->GetBlockData(brush->GetSelectedIndex());
	if (data != nullptr)
	{
		data->SetCount(brush, count);
	}
}

void ADefaultPlayerController::KOTC_DebugDrawTrace(const bool& enable)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->GetPrimaryBrush()->m_bDebugRenderTrace = enable;
	}
}

void ADefaultPlayerController::KOTC_DebugDrawStructures(const bool& enable)
{
	ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
	if(gamemode != nullptr)
	{
		gamemode->GetStructureManager()->m_bDebugRenderStructure = enable;
	}
}
