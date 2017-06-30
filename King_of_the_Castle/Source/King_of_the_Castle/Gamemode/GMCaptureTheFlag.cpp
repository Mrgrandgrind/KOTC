// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMCaptureTheFlag.h"

#include "Character/PlayerCharacter.h"
#include "Construction/BlockEntity.h"
#include "Construction/Blocks/FlagBlock.h"

AGMCaptureTheFlag::AGMCaptureTheFlag() : m_FlagHolderTeam1(nullptr), m_FlagHolderTeam2(nullptr)
{
}

void AGMCaptureTheFlag::OnBlockPlace(ABlock* block, AActor* source)
{
	Super::OnBlockPlace(block, source);

	// If the placed block is a flag
	AFlagBlock *flag = Cast<AFlagBlock>(block);
	if (flag != nullptr)
	{
		// Set the flag to the proper team
		if (this->m_FlagHolderTeam1 == source)
		{
			flag->SetTeam(1);
			this->m_FlagHolderTeam1 = nullptr;

			UE_LOG(LogClass, Log, TEXT("Team 1 flag placed"));
		}
		else if (this->m_FlagHolderTeam2 == source)
		{
			flag->SetTeam(2);
			this->m_FlagHolderTeam2 = nullptr;

			UE_LOG(LogClass, Log, TEXT("Team 2 flag placed"));
		}
	}
}

void AGMCaptureTheFlag::OnBlockDestroy(ABlock* block, AActor* source)
{
	Super::OnBlockDestroy(block, source);
}

void AGMCaptureTheFlag::OnBlockPickup(ABlockEntity* block, APlayerCharacter* player, const int& newCount)
{
	Super::OnBlockPickup(block, player, newCount);

	// If the picked up block is a flag
	if (block->GetParentBlockNameId() == ID_FLAG_BLOCK)
	{
		if (block->GetTeam() == 1)
		{
			this->m_FlagHolderTeam1 = player;
		}
		else if (block->GetTeam() == 2)
		{
			this->m_FlagHolderTeam2 = player;
		}
		else
		{
			UE_LOG(LogClass, Error, TEXT("[GMCaptureTheFlag] Invalid block team"));
		}
	}
}

void AGMCaptureTheFlag::OnBlockDrop(ABlockEntity* block, APlayerCharacter* player, const int& newCount)
{
	Super::OnBlockDrop(block, player, newCount);

	// If the dropped block is a flag
	if (block->GetParentBlockNameId() == ID_FLAG_BLOCK)
	{
		if (block->GetTeam() == 1)
		{
			this->m_FlagHolderTeam1 = nullptr;
		}
		else if (block->GetTeam() == 2)
		{
			this->m_FlagHolderTeam2 = nullptr;
		}
		else
		{
			UE_LOG(LogClass, Error, TEXT("[GMCaptureTheFlag] Invalid block team"));
		}
	}
}
