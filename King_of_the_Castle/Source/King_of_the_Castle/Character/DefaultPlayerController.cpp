// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "DefaultPlayerController.h"

#include "Gamemode/BaseGameMode.h"
#include "Construction/BlockData.h"
#include "Construction/BlockEntity.h"
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
		character->OnStunned(0.01f, false, true);
	}
}

void ADefaultPlayerController::KOTC_Stun(const float& duration)
{
	APlayerCharacter *character = this->GetCharacter();
	if (character != nullptr)
	{
		character->OnStunned(duration, false, false);
	}
}

void ADefaultPlayerController::KOTC_SetTeam(const int32& team)
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
		character->SetBuildReach(reach <= 0 ? KOTC_CONSTRUCTION_BLOCK_REACH : reach);
	}
}

void ADefaultPlayerController::KOTC_SetBlockCount(const int32& count)
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

void ADefaultPlayerController::KOTC_DebugBreakBlocks()
{
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlock::StaticClass(), out);

	for (AActor *next : out)
	{
		if (next->IsA(ABlockEntity::StaticClass()))
		{
			continue;
		}
		Cast<ABlock>(next)->DestroyBlock();
	}
}

void ADefaultPlayerController::KOTC_DebugClearEntities()
{
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlockEntity::StaticClass(), out);

	for (AActor *next : out)
	{
		Cast<ABlockEntity>(next)->ForceDespawn();
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
