// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockStructureManager.h"

#include "Construction/Block.h"
#include "Construction/BlockEntity.h"

#include "DrawDebugHelpers.h"

#define OFFSET_OFFSET 5.0f // How far to go out of bounds of block when finding surrounding

// Sets default values
ABlockStructureManager::ABlockStructureManager()
{
	Super::RootComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("BlockStructureManager"));
}

// Called when the game starts or when spawned
void ABlockStructureManager::BeginPlay()
{
	Super::BeginPlay();
}

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
		DrawDebugLine(Super::GetWorld(), location, location + offset, FColor::Silver, false, 6.0f, 0, 4.0f);
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

FBlockStructure* ABlockStructureManager::MergeStructures(FBlockStructure *structureA, FBlockStructure *structureB)
{
	check(structureA != structureB);

	// Move all structureB blocks into structureA and update their reference
	for (ABlock *block : structureB->blocks)
	{
		structureA->blocks.Add(block);
		block->SetStructure(structureA);
	}

	// Delete structureB
	structureB->blocks.Empty();
	for (int i = 0; i < this->m_Structures.Num(); i++)
	{
		if (&this->m_Structures[i] == structureB)
		{
			this->m_Structures.RemoveAt(i);
			break;
		}
	}
	return structureA;
}

void ABlockStructureManager::ProcessDestroy(ABlock *block)
{
	// We need to remove the block from the structure
	FBlockStructure *structure = block->GetStructure();
	if (structure == nullptr)
	{
		return;
	}
	structure->blocks.Remove(block);

	// We now need to confirm that we didn't just split the structure into two structures..
}

FBlockStructure* ABlockStructureManager::ProcessCreate(ABlock *block)
{
	// Block entities are not a part of a structure and can be ignored
	if (block->IsA(ABlockEntity::StaticClass()) || block->GetStructure() != nullptr)
	{
		return nullptr;
	}

	TArray<ABlock*> surrounding = this->GetNeighbours(block);
	if (surrounding.Num() == 0)
	{
		UE_LOG_TEXT("Structure: New");
		// If there are no surrounding blocks, we can simply create a new structure
		FBlockStructure structure;
		structure.blocks.Add(block);

		this->m_Structures.Add(structure);
		return &this->m_Structures[this->m_Structures.Num() - 1];
	}

	// There are multiple surrounding blocks. Let's get all their structures to see whats up.
	TArray<FBlockStructure*> structures;
	for (ABlock *neighbour : surrounding)
	{
		FBlockStructure *structure = neighbour->GetStructure();
		if (structure == nullptr)
		{
			// We shouldn't get any nullptr structures
			UE_LOG(LogClass, Error, TEXT("[WARNING] We found a block with no structure! BlockName: %s"), *block->GetName());
			continue;
		}
		if (!structures.Contains(structure))
		{
			structures.Add(structure);
		}
	}

	if (structures.Num() == 0)
	{
		// Our surrounding neighbours had no structures in them. 
		// This will happen for pre-placed blocks if they are not created by the gamemode using #ProcessPreplaced()
		UE_LOG_TEXT("Structure: Doesn't exist");
		return nullptr;
	}
	else if (structures.Num() == 1)
	{
		UE_LOG_TEXT("Structure: One surrounding. Adding to existing.");
		// If we only have one structure surrounding then we can add our block to said structure.
		structures[0]->blocks.Add(block);
		return structures[0];
	}
	else
	{
		UE_LOG_TEXT("Structure: Multiple structures. Merging.");
		// We are next to multiple structures. We are going to have to merge them into one.
		FBlockStructure *structure = structures[0]; // The chosen structure. We will merge the remaining into here.
		for (int i = 1; i < structures.Num(); i++)
		{
			this->MergeStructures(structure, structures[i]);
		}
		return structure;
	}
	return nullptr; // Can't get here
}

void ABlockStructureManager::AddNeighbours(ABlock *block, FBlockStructure *structure, TArray<AActor*>& outArray)
{
	if (block == nullptr || !outArray.Contains(block))
	{
		return;
	}
	outArray.Remove(block);
	structure->blocks.Add(block);
	block->SetStructure(structure);
	for (ABlock *next : this->GetNeighbours(block))
	{
		this->AddNeighbours(next, structure, outArray);
	}
}

void ABlockStructureManager::ProcessPreplaced()
{
	// Get all blocks on the map
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlock::StaticClass(), out);

	while (out.Num() > 0)
	{
		this->m_Structures.Add(FBlockStructure());

		// Call a recursive method that will add all neighbours to the structure recursively until there are none left.
		FBlockStructure *structure = &this->m_Structures[this->m_Structures.Num() - 1];
		this->AddNeighbours(Cast<ABlock>(out[0]), structure, out);
	}
	// When we get out of the while loop, every block on the map should be assigned a structure
}