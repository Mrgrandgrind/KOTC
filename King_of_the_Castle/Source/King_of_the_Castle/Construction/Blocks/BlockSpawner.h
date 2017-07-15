// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Construction/Block.h"
#include "BlockSpawner.generated.h"

#define ID_SPAWNER_BLOCK FName("SpawnerBlock")

UCLASS()
class KING_OF_THE_CASTLE_API ABlockSpawner : public ABlock
{
	GENERATED_BODY()
	
public:
	ABlockSpawner();

	virtual void Tick(float delta) override;

	virtual bool IsDestructable() const { return false; }

	virtual FName GetNameId() override { return ID_SPAWNER_BLOCK; }

private:
	float m_SpawnCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity", meta = (AllowPrivateAccess = "true", DisplayName = "Spawn Rate (seconds)"))
	float m_SpawnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity", meta = (AllowPrivateAccess = "true", DisplayName = "Entity Life Time"))
	float m_EntityLifeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (AllowPrivateAccess = "true", DisplayName = "Impulse Strength Min"))
	float m_ImpulseStrengthMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (AllowPrivateAccess = "true", DisplayName = "Impulse Strength Max"))
	float m_ImpulseStrengthMax;
};
