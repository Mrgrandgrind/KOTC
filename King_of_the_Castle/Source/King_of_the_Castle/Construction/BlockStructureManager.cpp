// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockStructureManager.h"

#include "Construction/Block.h"
#include "Construction/BlockEntity.h"

#include "DrawDebugHelpers.h"

#define OFFSET_OFFSET 6.0f // How far to go out of bounds of block when finding surrounding
#define DEBUG_DURATION 2.5f

// Sets default values
ABlockStructureManager::ABlockStructureManager()
{
	Super::RootComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("BlockStructureManager"));
}

#if WITH_EDITOR
void ABlockStructureManager::DrawDebugStructure(FStructureMeta* meta, FColor color) const
{
	if(!this->m_bDebugRenderStructure)
	{
		return;
	}
	const FBlockStructure& structure = this->m_Structures[meta->index];
	for(int i = 0; i < structure.blocks.Num(); i++)
	{
		this->DrawDebugBlock(structure.blocks[i], color);
	}
}

void ABlockStructureManager::DrawDebugBlock(ABlock* block, FColor color) const
{
	if (!this->m_bDebugRenderStructure)
	{
		return;
	}
	FVector origin, extent, loc = block->GetActorLocation();
	block->GetActorBounds(true, origin, extent);
	extent *= 1.02f;
	
	FColor newColor = FColor(color.R, color.G, color.B, 10);
	DrawDebugSolidBox(block->GetWorld(), loc, extent, newColor, true, DEBUG_DURATION);
}
#endif

TArray<ABlock*> ABlockStructureManager::GetNeighbours(ABlock *block) const
{
	// Find surrounding blocks
	TArray<ABlock*> surrounding;

	FVector location = block->GetActorLocation(), origin, extent;
	block->GetActorBounds(true, origin, extent);

	extent += FVector(OFFSET_OFFSET);

	FHitResult result;
	FCollisionQueryParams params;
	params.AddIgnoredActor(block);

	// Loop through the surrounding offsets and line trace to find the surrounding blocks
	for (const FVector& offset : {
		FVector(extent.X, 0.0f, 0.0f), FVector(-extent.X, 0.0f, 0.0f),
		FVector(0.0f, extent.Y, 0.0f), FVector(0.0f, -extent.Y, 0.0f),
		FVector(0.0f, 0.0f, extent.Z), FVector(0.0f, 0.0f, -extent.Z) })
	{
		// Line trace to find surrounding
		Super::GetWorld()->LineTraceSingleByChannel(result, location,
			location + offset, ECollisionChannel::ECC_WorldDynamic, params);
#if WITH_EDITOR
		if(this->m_bDebugRenderStructure)
		{
			DrawDebugLine(Super::GetWorld(), location, location + offset, FColor::Silver, false, DEBUG_DURATION, 0, 4.0f);
		}
#endif
		if (!result.IsValidBlockingHit())
		{
			continue;
		}
		ABlock *neighbour = Cast<ABlock>(result.GetActor());
		if (neighbour != nullptr)
		{
			surrounding.Add(neighbour);
		}
	}
	// When here, we should have the surrounding blocks. 
	return surrounding;
}

void ABlockStructureManager::MergeIntoA(FStructureMeta *metaA, FStructureMeta *metaB)
{
	check(metaA != metaB);
	check(metaA != nullptr && metaB != nullptr);

	FBlockStructure *structureA = &this->GetStructure(metaA->index), *structureB = &this->GetStructure(metaB->index);

	// We will be deleting metaB so lets just pre-emptively let the manager know we are no longer using its index
	this->m_StructureFreeIndex.Add(metaB->index);

	// Move all structureB blocks into structureA and update their reference
	for (ABlock *block : structureB->blocks)
	{
		structureA->blocks.Add(block);
		block->GetStructureMeta().index = metaA->index;
	}

	// Delete structureB
	structureB->blocks.Empty();
}

void ABlockStructureManager::ProcessDestroy(ABlock *block)
{
	// We need to remove the block from the structure
	FStructureMeta& meta = block->GetStructureMeta();
	if (meta.index == -1)
	{
		return;
	}
	FBlockStructure& structure = this->GetStructure(meta.index);
	structure.blocks.Remove(block);

	// If structure is dead, let it be overwritten
	if(structure.blocks.Num() == 0)
	{
		this->m_StructureFreeIndex.Add(meta.index);
	}
	meta.index = -1;

	// We now need to confirm that we didn't just split the structure into two structures..
}

void ABlockStructureManager::ProcessCreate(ABlock *block)
{
	// Block entities are not a part of a structure and can be ignored
	if (block->IsA(ABlockEntity::StaticClass()))
	{
		return;
	}

	TArray<ABlock*> surrounding = this->GetNeighbours(block);
	if (surrounding.Num() == 0)
	{
		//UE_LOG_TEXT("Structure: New");
		// If there are no surrounding blocks, we can simply create a new structure
		FBlockStructure structure;
		structure.blocks.Add(block);

		this->AddStructure(structure);
		block->GetStructureMeta().index = this->m_Structures.Num() - 1;
		return;
	}

	// There are multiple surrounding blocks. Let's get all their structures to see whats up.
	TArray<FStructureMeta*> metas;
	for (ABlock *neighbour : surrounding)
	{
		FStructureMeta *meta = &neighbour->GetStructureMeta();
		if (meta == nullptr || meta->index == -1)
		{
			// We shouldn't get any nullptr structures
			UE_LOG(LogClass, Error, TEXT("[WARNING] We found a block with no structure! BlockName: %s"), *block->GetName());
			continue;
		}
		if (!metas.ContainsByPredicate([meta](FStructureMeta *element) { return element->index == meta->index; }))
		{
			metas.Add(meta);
		}
	}

	if (metas.Num() == 0)
	{
		// Our surrounding neighbours had no structures in them. 
		// This will happen for pre-placed blocks if they are not created by the gamemode using #ProcessPreplaced()
		UE_LOG(LogClass, Error, TEXT("Structure: Doesn't exist"));
#if WITH_EDITOR
		this->DrawDebugBlock(block, FColor::Red);
#endif
	}
	else if (metas.Num() == 1)
	{
		//UE_LOG_TEXT("Structure: One surrounding. Adding to existing.");
		// If we only have one structure surrounding then we can add our block to said structure.
		this->GetStructure(metas[0]->index).blocks.Add(block);
		block->GetStructureMeta().index = metas[0]->index;
#if WITH_EDITOR
		this->DrawDebugStructure(metas[0], FColor::White);
#endif
	}
	else
	{
		//UE_LOG_TEXT("Structure: Multiple structures. Merging.");
		// We are next to multiple structures. We are going to have to merge them into one.
		FStructureMeta *meta = metas[0]; // The chosen structure. We will merge the remaining into here.
#if WITH_EDITOR
		this->DrawDebugStructure(meta, FColor::Blue);
		this->DrawDebugBlock(block, FColor::Green);
#endif
		for (int i = 1; i < metas.Num(); i++)
		{
#if WITH_EDITOR
			this->DrawDebugStructure(metas[i], FColor::Cyan);
#endif
			this->MergeIntoA(meta, metas[i]);
		}
		this->GetStructure(meta->index).blocks.Add(block);
		block->GetStructureMeta().index = meta->index;
	}
}

void ABlockStructureManager::AddNeighbours(ABlock *block, const int& structureIndex, TArray<AActor*>& outArray)
{
	if (block == nullptr)
	{
		return;
	}
	FBlockStructure& structure = this->GetStructure(structureIndex);
	outArray.Remove(block);
	structure.blocks.Add(block);
	block->GetStructureMeta().index = structureIndex;
	for (ABlock *next : this->GetNeighbours(block))
	{
		if(!outArray.Contains(next))
		{
			continue;
		}
		this->AddNeighbours(next, structureIndex, outArray);
	}
}

void ABlockStructureManager::ProcessPreplaced()
{
	// Get all blocks on the map
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlock::StaticClass(), out);

	while (out.Num() > 0)
	{
		FBlockStructure structure;
		int index = this->AddStructure(structure);

		// Call a recursive method that will add all neighbours to the structure recursively until there are none left.
		this->AddNeighbours(Cast<ABlock>(out[0]), index, out);
	}
	// When we get out of the while loop, every block on the map should be assigned a structure
}