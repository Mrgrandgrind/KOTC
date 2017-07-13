// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "Prefab.h"

#include "Construction/Block.h"

#define DEFAULT_GRID_SIZE 150.0f, 150.0f, 150.0f

APrefab::APrefab() : m_GridSize(DEFAULT_GRID_SIZE)
{
}

void APrefab::BeginPlay()
{
	Super::BeginPlay();

	check(this->m_GridSize.X > 0 && this->m_GridSize.Y > 0 && this->m_GridSize.Z > 0);
	this->PopulateBlocks();
}

FVector APrefab::GetPerfabLocation(const FIntVector& offset) const
{
	return FVector(this->m_GridSize.X * offset.X, this->m_GridSize.Y
		* offset.Y, this->m_GridSize.Z * offset.Z);
}

void APrefab::PopulateBlocks()
{
	if (Super::GetRootComponent() == nullptr)
	{
		return;
	}

	this->m_Blocks.Empty();

	// It is expected that the prefab has been built in the blueprint.
	// We must construct the prefab from the blocks. We need to be able to construct a brush from this data.
	for (USceneComponent *attached : Super::GetRootComponent()->GetAttachChildren())
	{
		if (!attached->IsA(UChildActorComponent::StaticClass()))
		{
			UE_LOG(LogClass, Error, TEXT("[Prefab] Invalid component attached to prefab. You can only add block actors."));
			continue;
		}
		ABlock *block = Cast<ABlock>(((UChildActorComponent*)attached)->GetChildActor());
		if (block == nullptr)
		{
			UE_LOG(LogClass, Error, TEXT("[Prefab] Invalid actor attached to prefab. You can only add block actors."));
			continue;
		}
		const FVector& location = attached->GetRelativeTransform().GetLocation();

		FPrefabBlock pb;
		pb.id = block->GetNameId();
		pb.offset.X = int(location.X / this->m_GridSize.X);
		pb.offset.Y = int(location.Y / this->m_GridSize.Y);
		pb.offset.Z = int(location.Z / this->m_GridSize.Z);

		this->m_Blocks.Add(pb);
	}
}

#if WITH_EDITOR
void APrefab::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(APrefab, m_bDebugSnapPositions))
	{
		if (Super::GetRootComponent() != nullptr)
		{
			// Root component can be nullptr for some reason (probably default object)
			for (USceneComponent *attached : Super::GetRootComponent()->GetAttachChildren())
			{
				UChildActorComponent *component = Cast<UChildActorComponent>(attached);
				if (component == nullptr)
				{
					continue;
				}
				if (component->GetChildActorClass()->IsChildOf(ABlock::StaticClass()))
				{
					const FVector& location = attached->GetRelativeTransform().GetLocation();

					FIntVector offset;
					offset.X = int(location.X / this->m_GridSize.X);
					offset.Y = int(location.Y / this->m_GridSize.Y);
					offset.Z = int(location.Z / this->m_GridSize.Z);

					attached->SetRelativeLocation(this->GetPerfabLocation(offset) + FVector(0.0f, 0.0f, this->m_GridSize.Z) / 2.0f);
				}
			}
		}
		this->m_bDebugSnapPositions = false;
	}
}
#endif
