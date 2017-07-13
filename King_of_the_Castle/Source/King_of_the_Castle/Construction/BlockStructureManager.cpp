// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockStructureManager.h"

#include "Construction/Block.h"
#include "Construction/BlockEntity.h"

#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Runtime/Engine/Classes/Components/ModelComponent.h"

#define OFFSET_OFFSET 6.0f // How far to go out of bounds of block when finding surrounding
#define DEBUG_DURATION 2.5f

#define PHYSICS_DELAY 0.25f
#define PHYSICS_DROP_OFFSET 80.0f

//#define ENTITY_DELAY_DIST_MULTIPLIER 0.0009f
#define ENTITY_DELAY_TIME_PER_NEXT 0.12f
#define ENTITY_DELAY_TIME_OFFSET (ENTITY_DELAY_TIME_PER_NEXT * 0.75f)

// Sets default values
ABlockStructureManager::ABlockStructureManager()
{
	Super::RootComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("BlockStructureManager"));

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABlockStructureManager::BeginPlay()
{
	Super::BeginPlay();

	this->ProcessPreplaced();
}

void ABlockStructureManager::Tick(float delta)
{
	Super::Tick(delta);

	if (this->m_PhysicsBlocks.Num() == 0)
	{
		return;
	}
	for (int i = this->m_PhysicsBlocks.Num() - 1; i >= 0; i--)
	{
		FPhysicsBlock& physics = this->m_PhysicsBlocks[i];
		switch (this->m_DropMode)
		{
			//case EDropMode::Regular: {
			//	if (physics.block->GetVelocity().IsNearlyZero())
			//	{
			//		physics.counter += delta;
			//		if (physics.counter >= PHYSICS_DELAY)
			//		{
			//			physics.block->GetMesh()->SetSimulatePhysics(false);
			//			this->m_PhysicsBlocks.RemoveAt(i);
			//		}
			//	}
			//	else
			//	{
			//		physics.counter = 0.0f;
			//	}
			//	break;
			//}
			case EDropMode::EntityDelay: {
				physics.counter -= delta;
				if (physics.counter <= 0.0f)
				{
					this->m_Structures[physics.block->GetStructureMeta().index].blocks.Remove(physics.block);
					physics.block->GetStructureMeta().index = -1;

					this->DropBlock(physics.block);
					this->m_PhysicsBlocks.RemoveAt(i);
				}
				break;
			}
			// We should never hit this case but if we do 
			// we will just discard the data so we can stop trying to process it
			default: this->m_PhysicsBlocks.RemoveAt(i);
		}
	}
}

bool ABlockStructureManager::IsSupport(class ABlock *block)
{
	FVector location = block->GetActorLocation(), origin, extent;
	block->GetActorBounds(true, origin, extent);

	// Check for ground first
	FHitResult result;
	Super::GetWorld()->LineTraceSingleByChannel(result, location, location
		+ FVector(0.0f, 0.0f, -extent.Z - OFFSET_OFFSET), ECollisionChannel::ECC_WorldDynamic);

	if (result.IsValidBlockingHit() && Cast<AStaticMeshActor>(result.GetActor()) != nullptr)
	{
		return true;
	}
	// If there's no ground, check the sides
	for (const FVector& offset : {
		FVector(extent.X + OFFSET_OFFSET, 0.0f, 0.0f), FVector(-extent.X - OFFSET_OFFSET, 0.0f, 0.0f),
		FVector(0.0f, extent.Y + OFFSET_OFFSET, 0.0f), FVector(0.0f, -extent.Y - OFFSET_OFFSET, 0.0f) })
	{
		Super::GetWorld()->LineTraceSingleByChannel(result, location, location + offset, ECollisionChannel::ECC_WorldDynamic);

		if (result.IsValidBlockingHit() && Cast<UModelComponent>(result.GetComponent()) != nullptr)
		{
			return true;
		}
	}
	return false;
}

void ABlockStructureManager::DropBlock(ABlock *block)
{
	for (ABlockEntity *entity : block->DropBlock(nullptr, true))
	{
		if (entity == nullptr)
		{
			continue;
		}
		FVector rand = (FVector(FMath::FRand(), FMath::FRand(),
			FMath::FRand()) - FVector(0.5f)) * PHYSICS_DROP_OFFSET;
		rand.Z = -FMath::Abs(rand.Z);
		((UPrimitiveComponent*)entity->GetRootComponent())->AddImpulse(rand, NAME_None, true);
	}
}

void ABlockStructureManager::CheckStructureSupport(FBlockStructure& structure)
{
	// Floating mode does not require support
	if (this->m_DropMode == EDropMode::Floating)
	{
		return;
	}
	bool supported = false;
	for (int i = 0; i < structure.blocks.Num() && !supported; i++)
	{
		supported = structure.blocks[i]->GetStructureMeta().isSupport;
	}
	if (!supported)
	{
		//UE_LOG_TEXT("Structure: Support was removed");
		// If there is no longer any support blocks in the structure then we need to enable gravity
		for (int i = structure.blocks.Num() - 1; i >= 0; i--)
		{
			ABlock *block = structure.blocks[i];
			if (block->GetStructureMeta().isDestroyed)
			{
				continue;
			}
			switch (this->m_DropMode)
			{
				//case EDropMode::Regular:
				//	block->GetMesh()->SetSimulatePhysics(true);
				//	this->m_PhysicsBlocks.Add({ block });
				//	break;
				case EDropMode::EntityInsta: {
					block->GetStructureMeta().index = -1;
					structure.blocks.RemoveAt(i);
					this->DropBlock(block);
					break;
				}
				case EDropMode::EntityDelay: {
					//float distance = FVector::Dist(block->GetActorLocation(), structure.lastSupport);
					//this->m_PhysicsBlocks.Add({ block, distance * ENTITY_DELAY_DIST_MULTIPLIER });

					float distance = this->GeneratePath(block, structure.lastSupport)
						.searched.Num() * ENTITY_DELAY_TIME_PER_NEXT;
					//distance += ENTITY_DELAY_TIME_OFFSET * FMath::FRand() - ENTITY_DELAY_TIME_OFFSET / 2.0f;
					this->m_PhysicsBlocks.Add({ block, distance });

					// Must be set after generating the path
					block->GetStructureMeta().isDestroyed = true;
					break;
				}
			}
		}
	}
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

void ABlockStructureManager::AddNeighbours(ABlock *block, const int& structureIndex, TArray<AActor*>& outArray)
{
	if (block == nullptr)
	{
		return;
	}
	FBlockStructure& structure = this->GetStructure(structureIndex);
	outArray.Remove(block);
	if (!structure.blocks.Contains(block))
	{
		structure.blocks.Add(block);
		block->GetStructureMeta().index = structureIndex;
		block->GetStructureMeta().isSupport = this->IsSupport(block);
	}
	for (ABlock *next : this->GetNeighbours(block))
	{
		if (!outArray.Contains(next))
		{
			continue;
		}
		this->AddNeighbours(next, structureIndex, outArray);
	}
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
#if WITH_EDITOR
		if (this->m_bDebugRenderStructure)
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

FBlockPath ABlockStructureManager::GeneratePath(ABlock *from, ABlock *to, ABlock *ignored) const
{
	struct Node
	{
		Node(ABlock *block) : block(block), parentIdx(-1) { }

		float h;
		ABlock *block;
		int parentIdx; //node index

		Node& CalculateHeuristics(ABlock *end)
		{
			this->h = (end->GetActorLocation() - this->block->GetActorLocation()).Size();
			return *this;
		}
	};
	auto ArrayContains = [](TArray<int>& indices, TArray<Node>& nodes, ABlock *block)
	{
		for (int i = 0; i < indices.Num(); i++)
		{
			if (nodes[indices[i]].block == block)
			{
				return true;
			}
		}
		return false;
	};

	TArray<Node> nodes; // To avoid memory access issues we need somewhere to place our nodes
	TArray<int> open, closed; //indices into nodes
	nodes.Add(Node(from).CalculateHeuristics(to));
	open.Add(nodes.Num() - 1);

	Node *endNode = nullptr;
	while (open.Num() > 0)
	{
		// Get lowest heuristics value
		int openIndex = -1, nodeIndex = -1;
		Node *next = nullptr;
		for (int j = 0; j < open.Num(); j++)
		{
			if (next == nullptr || nodes[open[j]].h < next->h)
			{
				next = &nodes[nodeIndex = open[openIndex = j]];
			}
		}
		check(next != nullptr);
		// Check to see if we're done
		if (next->block == to)
		{
			endNode = &nodes[nodeIndex];
			break;
		}
		closed.Add(nodeIndex);
		open.RemoveAt(openIndex);

		TArray<ABlock*> nextNeighbours = this->GetNeighbours(next->block);
		for (int j = 0; j < nextNeighbours.Num(); j++) // for some reason for-each gets compile error
		{
			ABlock *nextNeighbour = nextNeighbours[j];
			if (nextNeighbour == ignored || ArrayContains(closed, nodes, nextNeighbour))
			{
				continue;
			}
			if (!ArrayContains(open, nodes, nextNeighbour))
			{
				Node node = Node(nextNeighbour).CalculateHeuristics(to);
				node.parentIdx = nodeIndex;
				nodes.Add(node);
				open.Add(nodes.Num() - 1);
			}
		}
	}
	FBlockPath path;
	for (const int& idx : closed)
	{
		path.searched.Add(nodes[idx].block);
	}
	if (endNode == nullptr)
	{
		path.valid = false;
	}
	else
	{
		path.valid = true;
		do
		{
			path.path.Add(endNode->block);

			const int& idx = endNode->parentIdx;
			if (idx == -1)
			{
				break;
			}
			endNode = &nodes[idx];
		} while (true);
	}
	return path;
}

void ABlockStructureManager::ProcessPreplaced()
{
	// Get all blocks on the map
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlock::StaticClass(), out);

	while (out.Num() > 0)
	{
		int index = this->AddStructure(FBlockStructure());

		// Call a recursive method that will add all neighbours to the structure recursively until there are none left.
		this->AddNeighbours(Cast<ABlock>(out[0]), index, out);
	}
	// When we get out of the while loop, every block on the map should be assigned a structure
}

void ABlockStructureManager::ProcessCreate(ABlock *block)
{
	// Block entities and blocks already with a structure can be ignored
	if (block->IsA(ABlockEntity::StaticClass()) || block->GetStructureMeta().index != -1)
	{
		return;
	}
	check(!block->GetStructureMeta().isDestroyed);

	// Start by determining if this block is on the ground and can be used as a support
	block->GetStructureMeta().isSupport = this->IsSupport(block);

	TArray<ABlock*> surrounding = this->GetNeighbours(block);
	if (surrounding.Num() == 0)
	{
		//UE_LOG_TEXT("Structure: New");
		// If there are no surrounding blocks, we can simply create a new structure
		FBlockStructure structure;
		structure.blocks.Add(block);

		block->GetStructureMeta().index = this->AddStructure(structure);
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
		if (meta->isDestroyed)
		{
			// Don't interact with blocks flagged to be removed
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

void ABlockStructureManager::ProcessDestroy(ABlock *block)
{
	// We need to remove the block from the structure
	FStructureMeta& meta = block->GetStructureMeta();
	if (meta.index == -1 || meta.isDestroyed)
	{
		return;
	}
	FBlockStructure& structure = this->GetStructure(meta.index);
	structure.blocks.Remove(block);

	if (meta.isSupport)
	{
		structure.lastSupport = block;
	}

	// If structure is dead, let it be overwritten
	if (structure.blocks.Num() == 0)
	{
		this->m_StructureFreeIndex.Add(meta.index);
	}
	else
	{
		// We are now going to pathfind (A*) from neighbour to neighbour to ensure
		// that the structure is still whole after removing this block
		TArray<ABlock*> neighbours = this->GetNeighbours(block);
		if (neighbours.Num() == 1)
		{
			this->CheckStructureSupport(structure);
		}
		else for (int i = 1; i < neighbours.Num(); i++)
		{
			FBlockPath path = this->GeneratePath(neighbours[i - 1], neighbours[i], block);
			if (path.valid)
			{
				//UE_LOG_TEXT("Structure: No structure change detected")
				// If there's a path, all is good and the structure is still connected.
				// We don't have to do anything more.
				this->CheckStructureSupport(structure);
				continue;
			}
			else
			{
				//UE_LOG_TEXT("Structure: Structure split detected. Unmerging.")
				// If there's no path it means our structure has been split.
				// Luckily we can use the pathfinding data to know how to split the structure.
				// The closed list contains all blocks in one part of the structure.
				int index = this->AddStructure(FBlockStructure());

				for (int j = 0; j < path.searched.Num(); j++)
				{
					ABlock *next = path.searched[j];
					if (next == block)
					{
						continue;
					}
					structure.blocks.Remove(next);
					this->m_Structures[index].blocks.Add(next);
					next->GetStructureMeta().index = index;
				}
				structure.lastSupport = this->m_Structures[index].lastSupport = block;
				this->CheckStructureSupport(structure);
				this->CheckStructureSupport(this->m_Structures[index]);
#if WITH_EDITOR
				DrawDebugStructure(structure, FColor::Cyan);
				DrawDebugStructure(this->m_Structures[index], FColor::Blue);
#endif
			}
		}
	}
	// Reset index for no particular reason
	meta.index = -1;
}

#if WITH_EDITOR
void ABlockStructureManager::DrawDebugStructure(FStructureMeta* meta, const FColor& color) const
{
	this->DrawDebugStructure(this->m_Structures[meta->index], color);
}

void ABlockStructureManager::DrawDebugStructure(const FBlockStructure& structure, const FColor& color) const
{
	if (!this->m_bDebugRenderStructure)
	{
		return;
	}
	for (int i = 0; i < structure.blocks.Num(); i++)
	{
		this->DrawDebugBlock(structure.blocks[i], color);
	}
}

void ABlockStructureManager::DrawDebugBlock(ABlock* block, const FColor& color) const
{
	if (!this->m_bDebugRenderStructure)
	{
		return;
	}
	FVector origin, extent, loc = block->GetActorLocation();
	block->GetActorBounds(true, origin, extent);
	extent *= 1.02f;

	FColor newColor = block->GetStructureMeta().isSupport ? 
		FColor(200, 0, 200, 20) : FColor(color.R, color.G, color.B, 10);
	DrawDebugSolidBox(block->GetWorld(), loc, extent, newColor, true, DEBUG_DURATION);
}
#endif