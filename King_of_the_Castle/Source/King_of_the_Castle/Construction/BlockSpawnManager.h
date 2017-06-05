// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Block.h"
#include "GameFramework/Actor.h"
#include "BlockSpawnManager.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ABlockSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:
	ABlockSpawnManager();

	virtual void BeginPlay() override;

	virtual void Tick(float delta) override;

	bool SpawnBlock(TSubclassOf<class ABlock> type);

	void SpawnGold(const int& count) { this->m_GoldBlocks += count; }

	void SpawnBasic(const int& count) { this->m_BasicBlocks += count; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Spawns", meta = (AllowPrivateAccess = "true", DisplayName = "Basic Blocks"))
	int m_BasicBlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Spawns", meta = (AllowPrivateAccess = "true", DisplayName = "Gold Blocks"))
	int m_GoldBlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Delay", meta = (AllowPrivateAccess = "true", DisplayName = "Spawn Delay"))
	float m_SpawnDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delay", meta = (AllowPrivateAccess = "true", DisplayName = "Delay Counter"))
	float m_DelayCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Areas", meta = (AllowPrivateAccess = "true", DisplayName = "Spawn Areas"))
	TArray<class ABlockSpawnArea*> m_SpawnAreas;

	TSubclassOf<class ABlock> m_BasicType, m_GoldType;
};
