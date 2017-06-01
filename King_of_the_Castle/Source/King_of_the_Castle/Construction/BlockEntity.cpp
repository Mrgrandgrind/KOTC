#include "King_of_the_Castle.h"

#include "../Character/PlayerCharacter.h"
#include "Runtime/Engine/Public/EngineUtils.h"

#include "BlockEntity.h"

#define SCALE_MULTIPLIER 0.5f
#define ATTRACTION_FORCE 50.0f
#define ATTRACTION_DISTANCE 680.0f
#define ATTRACTION_DISTANCE_SQUARED (ATTRACTION_DISTANCE * ATTRACTION_DISTANCE)

#define OWNERSHIP_DURATION 3.0f // block is owned by owner for 3 seconds

ABlockEntity::ABlockEntity() : m_bRestrictedPickup(true)
{
	FScriptDelegate sdb;
	sdb.BindUFunction(this, "BeginOverlap");
	Super::m_Mesh->OnComponentBeginOverlap.Add(sdb);

	FScriptDelegate sde;
	sde.BindUFunction(this, "EndOverlap");
	Super::m_Mesh->OnComponentEndOverlap.Add(sde);

	Super::m_Mesh->SetCollisionProfileName(TEXT("DroppedBlock"));

	Super::m_PointValue = 0;
	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABlockEntity::BeginPlay()
{
	Super::BeginPlay();

	Super::m_Mesh->SetMassOverrideInKg(NAME_None, 1.0f);
}

void ABlockEntity::SetTo(ABlock *block)
{
	Super::SetActorScale3D(block->GetActorScale3D() * SCALE_MULTIPLIER);
	Super::m_Mesh->SetStaticMesh(block->GetMesh()->StaticMesh);
	Super::m_Mesh->SetMaterial(0, block->GetMesh()->GetMaterial(0));

	this->m_ParentBlockNameId = block->GetNameId();
}

bool ABlockEntity::CanBePickedUp(APlayerCharacter *character) const
{
	//UCreateBrush *brush = character->GetCreateBrush();
	//int index = brush->GetIndexOf(this->m_ParentBlockType);
	//if (index == -1)
	//{
	//	return false;
	//}
	//UBlockData *data = brush->GetBlockData(index);
	//return (!this->m_bLimitedPickup && this->m_Owner != nullptr) || data->GetCount() < data->GetMaxCount();
	return false;
}

void ABlockEntity::Pickup(APlayerCharacter* character)
{
	//UCreateBrush *brush = character->GetCreateBrush();
	//UBlockData *data = brush->GetBlockData(brush->GetIndexOf(this->m_ParentBlockType));
	//if (data != nullptr)
	//{
	//	data->SetCount(brush, data->GetCount() + 1);
	//}

	//// Destroy this dropped block. Only happens if the block is a valid pickup.
	//Super::Destroy();
}

void ABlockEntity::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor == nullptr || OtherComp == nullptr)
	{
		return;
	}
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	// Check to see if the owner of this dropped block is the overlapping character
	if (this->m_Collisions.Contains(character) || (this->m_Owner != nullptr && character != this->m_Owner))
	{
		return;
	}
	// Check to see if we're overlapping with the capsule
	if (character != nullptr && OtherComp == character->GetCapsuleComponent())
	{
		this->m_Collisions.Add(character);
	}
}

void ABlockEntity::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	if (character != nullptr && this->m_Collisions.Contains(character))
	{
		this->m_Collisions.Remove(character);
	}
}

void ABlockEntity::Tick(float delta)
{
	Super::Tick(delta);

	// If this block has an owner, increase the timer. If timer is up, remove the owner
	if (this->m_Owner != nullptr && (this->m_Timer += delta) >= OWNERSHIP_DURATION)
	{
		this->m_Owner = nullptr;
		this->m_bRestrictedPickup = true;
	}

	// Get the closest player within ATTRACTION_DISTANCE
	APlayerCharacter *closest = nullptr;
	for (TActorIterator<AActor> itr(Super::GetWorld()); itr; ++itr)
	{
		APlayerCharacter *character = Cast<APlayerCharacter>(*itr);
		// Check to see if the character is this blocks owner. Or if this block has no owner.
		if (character != nullptr && (this->m_Owner == nullptr || character == this->m_Owner))
		{
			if (!this->CanBePickedUp(character))
			{
				continue;
			}
			// Check to see if the distance to this character is closer than existing
			float distance = character->GetSquaredDistanceTo(this);
			if (distance <= ATTRACTION_DISTANCE_SQUARED && (closest == nullptr
				|| distance < closest->GetSquaredDistanceTo(this)))
			{
				closest = character;
			}
		}
	}
	// If we found a valid character that we want this dropped block to move towards
	if (closest != nullptr)
	{
		if (this->m_Collisions.Contains(closest))
		{
			this->Pickup(closest);
		}
		else
		{
			UCapsuleComponent *capsule = closest->GetCapsuleComponent();
			float perc = closest->GetSquaredDistanceTo(this) / ATTRACTION_DISTANCE_SQUARED;
			FVector dir = (capsule->GetComponentLocation() - FVector(0.0f, 0.0f,
				capsule->Bounds.BoxExtent.Z * perc) - Super::GetActorLocation()).GetSafeNormal();

			Super::m_Mesh->AddImpulse(ATTRACTION_FORCE * (1.0f - perc) * dir, NAME_None, true);

			//DrawDebugLine(Super::GetWorld(), Super::GetActorLocation(), capsule->GetComponentLocation() - FVector(0.0f, 0.0f,
			//	capsule->Bounds.BoxExtent.Z * perc), FColor::Red, false, -1, 0, 4);
		}
	}
}

TArray<ABlockEntity*> ABlockEntity::SpawnBlockEntity(ABlock *block, AActor *source, const bool& restrictPickup)
{
	TArray<ABlockEntity*> blocks;

	TArray<ABlock*>& recipe = block->GetRecipe();
	if (recipe.Num() == 0)
	{
		recipe.Add(block);
	}
	for (int i = 0; i < recipe.Num(); i++)
	{
		ABlockEntity *entity = (ABlockEntity*)ABlock::SpawnBlock(block->GetWorld(), ABlockEntity::StaticClass(), block->GetTeam());
		if (entity != nullptr)
		{
			entity->SetBlockOwner(source);
			entity->SetTo(recipe[i]);
			entity->SetRestrictedPickup(restrictPickup);

			entity->SetActorLocation(block->GetActorLocation());
			entity->SetActorRotation(block->GetActorRotation());

			blocks.Add(entity);

			if (source == nullptr)
			{
				continue;
			}
			FVector origin, extent;
			block->GetActorBounds(false, origin, extent);

			// Distribute the dropped blocks evenly in an axis perpendicular to the the forward vector of the player and the z axis
			FVector direction;
			direction = (block->GetActorLocation() - source->GetActorLocation()).GetSafeNormal();
			direction = FVector::CrossProduct(FVector(0.0f, 0.0f, 1.0f), direction).GetSafeNormal();
			float size = (extent * direction).Size(), diff = (2.0f * i + 1.0f) / recipe.Num() - 1.0f;

			entity->SetActorLocation(block->GetActorLocation() + direction * diff * size);

			((UPrimitiveComponent*)entity->GetRootComponent())->AddImpulse((source->
				GetActorLocation() - block->GetActorLocation()) * 0.75f, NAME_None, true);
		}
	}
	return blocks;
}