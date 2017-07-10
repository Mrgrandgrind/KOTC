// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "DefaultPlayerController.h"

#include "Construction/BlockData.h"
#include "Character/PlayerCharacter.h"
#include "Construction/Brush/PrimaryBrush.h"

APlayerCharacter* ADefaultPlayerController::GetCharacter() const
{
	return Cast<APlayerCharacter>(Super::GetPawn());
}

void ADefaultPlayerController::KOTC_Stun()
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->Stun();
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