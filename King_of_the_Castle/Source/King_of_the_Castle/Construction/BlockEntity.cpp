#include "King_of_the_Castle.h"

#include "../Character/PlayerCharacter.h"

#include "BlockData.h"
#include "BlockEntity.h"

#include "Brush/PrimaryBrush.h"
#include "Gamemode/BaseGameMode.h"

#include "Runtime/Engine/Public/EngineUtils.h"

#define SCALE_DURATION 0.08f
#define SCALE_MULTIPLIER 0.5f

#define ATTRACTION_FORCE 50.0f
#define ATTRACTION_DISTANCE 680.0f
#define ATTRACTION_DISTANCE_SQUARED (ATTRACTION_DISTANCE * ATTRACTION_DISTANCE)

#define OWNERSHIP_DURATION 3.0f // block is owned by owner for 3 seconds

ABlockEntity::ABlockEntity() : m_bIgnoreOwner(false), m_bRestrictedPickup(true)
{
	FScriptDelegate sdb;
	sdb.BindUFunction(this, "BeginOverlap");
	Super::m_Mesh->OnComponentBeginOverlap.Add(sdb);

	FScriptDelegate sde;
	sde.BindUFunction(this, "EndOverlap");
	Super::m_Mesh->OnComponentEndOverlap.Add(sde);

	Super::m_Mesh->SetSimulatePhysics(true);
	Super::m_Mesh->SetCollisionProfileName(FName("BlockEntity"));

	Super::m_PointValue = 0;
	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABlockEntity::BeginPlay()
{
	Super::BeginPlay();

	Super::m_Mesh->SetMassOverrideInKg(NAME_None, 1.0f);

	FBodyInstance *body = this->GetMesh()->GetBodyInstance();
	if (body != nullptr)
	{
		body->SetDOFLock(EDOFMode::None);
	}
}

void ABlockEntity::SetTo(ABlock *block)
{
	//Super::SetActorScale3D(block->GetActorScale3D() * SCALE_MULTIPLIER);
	Super::SetActorScale3D(this->m_BaseScale = block->GetActorScale3D());
	Super::m_Mesh->SetStaticMesh(block->GetMesh()->GetStaticMesh());
	Super::m_Mesh->SetMaterial(0, block->GetMesh()->GetMaterial(0));

	this->m_DesiredScale = this->m_BaseScale * SCALE_MULTIPLIER;
	this->m_ParentBlockNameId = block->GetNameId();
}

bool ABlockEntity::CanBePickedUp(APlayerCharacter *character) const
{
	if(character->IsStunned() || (this->m_bIgnoreOwner && this->m_Owner == character))
	{
		return false;
	}
	UPrimaryBrush *brush = character->GetPrimaryBrush();
	int index = brush->GetIndexOf(this->m_ParentBlockNameId);
	if (index == -1)
	{
		return false;
	}
	UBlockData *data = brush->GetBlockData(index);
	return (!this->m_bRestrictedPickup && this->m_Owner != nullptr) || data->GetCount() < data->GetMaxCount();
}

void ABlockEntity::Pickup(APlayerCharacter* character)
{
	UPrimaryBrush *brush = character->GetPrimaryBrush();
	UBlockData *data = brush->GetBlockData(brush->GetIndexOf(this->m_ParentBlockNameId));
	if (data != nullptr)
	{
		data->SetCount(brush, data->GetCount() + 1);

		ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
		if(gamemode != nullptr)
		{
			gamemode->OnBlockPickup(this, character, data->GetCount());
		}
	}
	// Destroy this dropped block. Only happens if the block is a valid pickup.
	Super::Destroy();
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

	if (this->m_ScaleCounter < SCALE_DURATION)
	{
		this->m_ScaleCounter = FMath::Min(this->m_ScaleCounter + delta, SCALE_DURATION);

		float perc = FMath::Sin(HALF_PI * this->m_ScaleCounter / SCALE_DURATION);
		Super::SetActorScale3D(this->m_BaseScale + (this->m_DesiredScale - this->m_BaseScale) * perc);
	}

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
		if (character != nullptr && (this->m_Owner == nullptr || (this->m_bIgnoreOwner
			? character != this->m_Owner : character == this->m_Owner)))
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

TArray<ABlockEntity*> ABlockEntity::SpawnBlockEntity(ABlock* block, AActor* source, const bool& restrictPickup)
{
	UWorld *world = block->GetWorld();
	if (world == nullptr && source != nullptr)
	{
		world = source->GetWorld();
	}
	return ABlockEntity::SpawnBlockEntity(block, world, source, restrictPickup);
}

TArray<ABlockEntity*> ABlockEntity::SpawnBlockEntity(ABlock *block, UWorld* world, AActor *source, const bool& restrictPickup)
{
	TArray<ABlockEntity*> blocks;

	TArray<ABlock*>& recipe = block->GetRecipe();
	if (recipe.Num() == 0)
	{
		recipe.Add(block);
	}
	for (int i = 0; i < recipe.Num(); i++)
	{
		ABlockEntity *entity = (ABlockEntity*)ABlock::SpawnBlock(world, ABlockEntity::StaticClass(), block->GetTeam(), source);
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
