// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BlockStructureManager.generated.h"

struct FStructureMeta
{
	bool onGround;
};

USTRUCT()
struct FBlockStructure
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<class ABlock*> blocks;
};

UCLASS()
class KING_OF_THE_CASTLE_API ABlockStructureManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlockStructureManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ProcessDestroy(class ABlock *block);

	FBlockStructure* ProcessCreate(class ABlock *block);

	void ProcessPreplaced();

	FORCEINLINE const TArray<FBlockStructure>& GetStructures() const { return this->m_Structures; }

protected:
	void AddNeighbours(class ABlock *block, FBlockStructure *structure, TArray<AActor*>& outArray);

	TArray<class ABlock*> GetNeighbours(class ABlock *block) const;

	// Merge structureB into structureA. Returns structureA and destroys structureB.
	FBlockStructure* MergeStructures(FBlockStructure *structureA, FBlockStructure *structureB);

private:
	UPROPERTY()
	TArray<FBlockStructure> m_Structures;
};
