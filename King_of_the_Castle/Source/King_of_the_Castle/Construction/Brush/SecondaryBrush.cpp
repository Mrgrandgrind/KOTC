// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "SecondaryBrush.h"

#include "PrimaryBrush.h"
#include "Construction/Block.h"
#include "Construction/BlockData.h"
#include "Construction/BuildArea.h"
#include "Construction/Blocks/FlagBlock.h"
#include "Character/PlayerCharacter.h"

#include "Runtime/Engine/Classes/Engine/TextRenderActor.h"

#define COMBINE_DEAD_TIME 0.4f // How many seconds before the combining begins

#define BRUSH_DESTROY_COLOR FLinearColor(0.75f, 0.0f, 0.0f, 1.0f)

USecondaryBrush::USecondaryBrush()
{
}

void USecondaryBrush::BeginPlay()
{
	Super::BeginPlay(); 

	UMaterialInstanceDynamic *material = Super::GetMaterialDynamic();
	if (material != nullptr)
	{
		material->SetVectorParameterValue(BRUSH_COLOR_NAME, BRUSH_DESTROY_COLOR);
	}
}

TArray<ABlock*> USecondaryBrush::OnAction(ABuildArea* area, AActor* source)
{
	TArray<ABlock*> blocks;
	if (!Super::IsBrushVisible())
	{
		return blocks;
	}
	ABlock *block = Cast<ABlock>(Super::m_LastTrace.GetActor());
	if (block == nullptr || !block->IsDestructable()
		|| (block->IsA(AFlagBlock::StaticClass()) && block->GetTeam() == area->GetTeam()))
	{
		return blocks;
	}
	if (block->GetTeam() != *Super::m_Team)
	{
		return blocks;
	}
	// If we get here, the destroy is valid and we will turn the block into a dropped one
	block->DropBlock(source, false);
	blocks.Add(block);
	return blocks;
}

bool USecondaryBrush::OnMainCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& pre)
{
	if (!pre)
	{
		return false;
	}
	ABlock *block = Cast<ABlock>(result.GetActor());
	if (block != nullptr && area->GetGridCell(block->GetActorLocation(), Super::m_ActiveCell))
	{
		//// Method 1 - Works with a single call but has jitter
		//FVector diff = Super::GetComponentLocation() - Super::Bounds.Origin;
		//Super::SetWorldLocation(block->GetActorLocation() + diff);
		//Super::SetWorldRotation(block->GetActorRotation());
		////

		// Method 2 - Works but has two calls
		Super::SetWorldLocation(block->GetActorLocation());
		Super::SetWorldRotation(block->GetActorRotation());

		FVector diff = block->GetActorLocation() - Super::Bounds.Origin;
		Super::SetWorldLocation(Super::GetComponentLocation() + diff);
		//

		// Only show block count if primary brush isn't showing it
		APlayerCharacter *character = Cast<APlayerCharacter>(Super::GetOwner());
		if (character != nullptr)
		{
			UPrimaryBrush *brush = character->GetPrimaryBrush();
			Super::UpdateCountText(brush->GetBlockData(brush->GetSelectedIndex()));
			Super::m_TextActor->SetActorHiddenInGame(brush->IsBrushVisible());
		}

		show = true;
		return true;
	}
	show = false;
	return false;
}