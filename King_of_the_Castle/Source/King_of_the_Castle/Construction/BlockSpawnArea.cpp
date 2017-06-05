// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockSpawnArea.h"

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
	FVector extent = this->m_SpawnBox->GetScaledBoxExtent(), areaExtent = this->m_Area->GetScaledBoxExtent();
	FVector loc = (this->m_Area->GetComponentLocation() - areaExtent + extent)
		+ (areaExtent * 2.0f - extent * 2.0f) * FVector(FMath::FRand(), FMath::FRand(), FMath::FRand());
	this->m_SpawnBox->SetWorldRotation(FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f));
	this->m_SpawnBox->SetWorldLocation(loc);

	// Check for overlaps in the new position
	TArray<UPrimitiveComponent*> overlap;
	this->m_SpawnBox->GetOverlappingComponents(overlap);

	int count = 0;
	for (UPrimitiveComponent *next : overlap)
	{
		// Only collide with static mesh components
		if (next->IsA(UStaticMeshComponent::StaticClass()))
		{
			count++;
		}
	}
	if (count > 0)
	{
		// We are overlapping with a mesh. Our spawn position has failed.
		return false;
	}

	// We are in an empty position. Cast a ray down from the four corners of the box to check for a flat surface below to spawn on.
	FVector arr[] =
	{
		loc + FVector(extent.X, extent.Y, -extent.Z),
		loc + FVector(-extent.X, -extent.Y, -extent.Z),
		loc + FVector(-extent.X, extent.Y, -extent.Z),
		loc + FVector(extent.X, -extent.Y, -extent.Z)
	};
	for (int i = 0; i < 4; i++)
	{
		float zDiff = arr[i].Z - this->m_Area->GetComponentLocation().Z + areaExtent.Z;

		FHitResult result;
		Super::GetWorld()->LineTraceSingleByChannel(result, arr[i], arr[i] - FVector(0.0f, 0.0f, zDiff),
			ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
		//DrawDebugLine(Super::GetWorld(), arr[i], arr[i] - FVector(0.0f, 0.0f, zDiff), FColor::Red, true, 50, 0, 4);

		if (!result.IsValidBlockingHit())
		{
			// We didn't hit any ground that we can spawn on. Position failed.
			return false;
		}
		// Validate that all impact points have the same Z value. We use an optimization here and do it within the same array and loop.
		arr[i] = result.ImpactPoint;
		if (i != 0 && !FMath::IsNearlyEqual(arr[i].Z, arr[i - 1].Z))
		{
			// Uneven platform for spawning. We need a flat surface. Position failed.
			return false;
		}
	}

	// If we get here then all position tests passed. We can spawn a block at our random (grounded) position. 
	// Let the caller know the location and tell them the test was successful.
	out = FVector(loc.X, loc.Y, arr[0].Z + extent.Z);
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