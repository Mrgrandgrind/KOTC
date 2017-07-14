// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockSpawnArea.h"

#include "Block.h"
#include "BlockEntity.h"

#define SPAWN_Z_MAX_OFFSET 5.0f
#define SPAWN_BOX_UP_SCALE 1.75f

ABlockSpawnArea::ABlockSpawnArea()
{
	this->m_SpawnRating = 100.0f;
	this->m_SpawnBoxExtent = FVector(150.0f / 2.0f);

	USceneComponent *root = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("SpawnAreaRoot"));
	Super::RootComponent = root;

	this->m_Area = UObject::CreateDefaultSubobject<UBoxComponent>(TEXT("BoxArea"));
	this->m_Area->SetupAttachment(root);

	this->m_SpawnBox = UObject::CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	this->m_SpawnBox->SetBoxExtent(this->m_SpawnBoxExtent);
	this->m_SpawnBox->SetupAttachment(root);
	this->m_SpawnBox->SetVisibility(false);

	Super::PrimaryActorTick.bCanEverTick = false;
}

bool ABlockSpawnArea::TrySpawn(FVector& out) const
{
	// Place spawn block somewhere inside the spawn area
	this->m_SpawnBox->SetWorldScale3D(FVector(SPAWN_BOX_UP_SCALE));

	FVector extent = this->m_SpawnBox->GetScaledBoxExtent(), areaExtent = this->m_Area->GetScaledBoxExtent();
	FVector loc = (this->m_Area->GetComponentLocation() - areaExtent + extent)
		+ (areaExtent * 2.0f - extent * 2.0f) * FVector(FMath::FRand(), FMath::FRand(), FMath::FRand());
	//this->m_SpawnBox->SetWorldRotation(FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f));
	this->m_SpawnBox->SetWorldLocation(loc);

	// Check for overlaps in the new position
	TArray<UPrimitiveComponent*> overlap;
	this->m_SpawnBox->GetOverlappingComponents(overlap);

	for (UPrimitiveComponent *next : overlap)
	{
		ABlock *block = Cast<ABlock>(next->GetOwner());
		if (block != nullptr && !block->IsA(ABlockEntity::StaticClass()))
		{
			// If our spawn box has hit a block
			return this->TrySpawnNextTo(block, out);
		}
		if (next->IsA(UStaticMeshComponent::StaticClass()))
		{
			// Don't collide with static mesh components
			return false;
		}
	}

	// We are in an empty position. Cast a ray down from the four corners of the box to check for a flat surface below to spawn on.
	FVector arr[] =
	{
		loc + FVector(extent.X, extent.Y, -extent.Z),
		loc + FVector(-extent.X, -extent.Y, -extent.Z),
		loc + FVector(-extent.X, extent.Y, -extent.Z),
		loc + FVector(extent.X, -extent.Y, -extent.Z)
	};
	FHitResult result;
	for (int i = 0; i < 4; i++)
	{
		float zDiff = arr[i].Z - this->m_Area->GetComponentLocation().Z + areaExtent.Z;

		Super::GetWorld()->LineTraceSingleByChannel(result, arr[i], arr[i] - FVector(0.0f, 0.0f, zDiff),
			ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
		//DrawDebugLine(Super::GetWorld(), arr[i], arr[i] - FVector(0.0f, 0.0f, zDiff), FColor::Red, true, 50, 0, 4);

		if (!result.IsValidBlockingHit())
		{
			// We didn't hit any ground that we can spawn on. Position failed.
			return false;
		}
		// Store the impact point. We use an optimization here and do it within the same array and loop.
		arr[i] = result.ImpactPoint;
	}
	float zMax = arr[0].Z, zMin = zMax;
	for (int i = 1; i < 4; i++)
	{
		zMax = FMath::Max(arr[i].Z, zMax);
		zMin = FMath::Min(arr[i].Z, zMin);
	}
	if (zMax - zMin > SPAWN_Z_MAX_OFFSET)
	{
		// Uneven platform for spawning. We need a flatish surface. Position failed.
		return false;
	}

	// If we get here then all position tests passed. We can spawn a block at our random (grounded) position. 
	// Let the caller know the location and tell them the test was successful.
	out = FVector(loc.X, loc.Y, arr[0].Z + extent.Z / 2.0f);
	return true;
}

bool ABlockSpawnArea::TrySpawnNextTo(ABlock* block, FVector& out) const
{
	this->m_SpawnBox->SetWorldScale3D(FVector(1.0f));
	this->m_SpawnBox->SetWorldRotation(block->GetActorRotation());

	FVector extent = this->m_SpawnBox->GetScaledBoxExtent();

	// Get all the various offsets for all the positions around the block (except diagonal and -z)
	FVector offsets[] =
	{
		FVector(extent.X, 0.0f, 0.0f),
		FVector(-extent.X, 0.0f, 0.0f),
		FVector(0.0f, extent.Y, 0.0f),
		FVector(0.0f, -extent.Y, 0.0f),
		FVector(0.0f, 0.0f, extent.Z)
	};
	this->m_SpawnBox->SetWorldLocation(block->GetActorLocation() + offsets[FMath::RandRange(0, 4)] * 2.0f); //where 0 and 4 if offsets bounds

	// Check for overlaps in the new position
	TArray<UPrimitiveComponent*> overlap;
	this->m_SpawnBox->GetOverlappingComponents(overlap);

	for(UPrimitiveComponent *next : overlap)
	{
		if(next->GetOwner() == block)
		{
			continue;
		}
		if(next->IsA(UStaticMeshComponent::StaticClass()))
		{
			return false;
		}
	}
	FVector loc = this->m_SpawnBox->GetComponentLocation();

	// New position looks good to spawn. Run an additional check to stop blocks spawning mid-air. 
	// Add the offset to the Z so that we can still spawn next to blocks which are falling.
	FHitResult result;
	Super::GetWorld()->LineTraceSingleByChannel(result, loc, loc - FVector(0.0f, 0.0f, extent.Z * 2.0f + BRUSH_SPAWN_Z_OFFSET), 
		ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
	if(!result.IsValidBlockingHit())
	{
		// There's nothing below this spawn point. Lets not use it.
		return false;
	}

	// If the position is good, use it
	out = loc;
	return true;
}

#if WITH_EDITOR
void ABlockSpawnArea::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(ABlockSpawnArea, m_bDebugSetSpawn))
	{
		FVector out;
		this->TrySpawn(out);
		this->m_bDebugSetSpawn = false;
	}
	if (name == GET_MEMBER_NAME_CHECKED(ABlockSpawnArea, m_SpawnBoxExtent))
	{
		this->m_SpawnBox->SetBoxExtent(this->m_SpawnBoxExtent);
	}
}
#endif