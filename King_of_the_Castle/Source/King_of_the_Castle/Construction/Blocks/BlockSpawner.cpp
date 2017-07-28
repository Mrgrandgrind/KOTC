// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockSpawner.h"

#include "Construction/BlockEntity.h"
#include "Construction/Blocks/BasicBlock.h"

#define SPAWN_RATE 1.0f
#define SPAWNED_ENTITY_LIFE_TIME 16.0f // seconds

#define IMPULSE_STRENGTH_MAX 600.0f
#define IMPULSE_STRENGTH_MIN 400.0f
#define MAX_ANGLE_XY 90.0f

ABlockSpawner::ABlockSpawner() : m_SpawnRate(SPAWN_RATE), m_EntityLifeTime(SPAWNED_ENTITY_LIFE_TIME),
m_ImpulseStrengthMin(IMPULSE_STRENGTH_MIN), m_ImpulseStrengthMax(IMPULSE_STRENGTH_MAX)
{
	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABlockSpawner::Tick(float delta)
{
	Super::Tick(delta);

	this->m_SpawnCounter += delta;
	while (this->m_SpawnCounter > this->m_SpawnRate)
	{
		// If we need to spawn an entity, spawn one
		ABlock *blueprint = Cast<ABlock>(ABasicBlock::StaticClass()->GetDefaultObject());
		for (ABlockEntity *entity : ABlockEntity::SpawnBlockEntity(blueprint, Super::GetWorld(), nullptr))
		{
			entity->SkipSpawnAnimation();
			entity->SetLifeTime(this->m_EntityLifeTime);
			entity->SetActorLocation(Super::GetActorLocation());
			entity->SetActorRotation(FRotator(360.0f * FMath::FRand(),
				360.0f * FMath::FRand(), 360.0f * FMath::FRand()));
			
#define ANGLE -MAX_ANGLE_XY / 2.0f + MAX_ANGLE_XY * FMath::FRand()
#define IMPULSE this->m_ImpulseStrengthMin + (this->m_ImpulseStrengthMax - this->m_ImpulseStrengthMin) * FMath::FRand()
			FRotator rotation = FRotator(ANGLE, 360.0f * FMath::FRand(), ANGLE);
			FVector impulse = rotation.RotateVector(FVector(0.0f, 0.0f, IMPULSE));
			entity->GetMesh()->AddImpulse(impulse);
		}
		this->m_SpawnCounter -= this->m_SpawnRate;
	}
}



