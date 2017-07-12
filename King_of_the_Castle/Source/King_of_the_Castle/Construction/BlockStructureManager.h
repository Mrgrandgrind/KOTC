// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BlockStructureManager.generated.h"

UENUM()
enum class EDropMode : uint8
{
	Floating,
	EntityInsta

	//Regular,
	//EntityDelay
};

struct FPhysicsBlock
{
	class ABlock *block;
	float counter;
};

struct FStructureMeta
{
	int index = -1;
	bool isSupport;
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

	virtual void BeginPlay() override;

	virtual void Tick(float delta) override;

	bool IsOnGround(class ABlock *block);

	void ProcessDestroy(class ABlock *block);

	void ProcessCreate(class ABlock *block);

	FORCEINLINE const TArray<FBlockStructure>& GetStructures() const { return this->m_Structures; }

	FORCEINLINE FBlockStructure& GetStructure(const int& index) { return this->m_Structures[index]; }

	FORCEINLINE int AddStructure(FBlockStructure&& structure) 
	{
		FBlockStructure newStructure;
		newStructure.blocks.Append(structure.blocks);
		return this->AddStructure(newStructure);
	}

	FORCEINLINE int AddStructure(FBlockStructure& structure)
	{
		if(this->m_StructureFreeIndex.Num() > 0)
		{
			int index = this->m_StructureFreeIndex.Pop();
			this->m_Structures[index] = structure;
			return index;
		}
		return this->m_Structures.Add(structure);
	}

protected:
	void CheckStructureSupport(FBlockStructure& structure);

	void ProcessPreplaced();

#if WITH_EDITOR
	void DrawDebugStructure(FStructureMeta *meta, const FColor& color) const;

	void DrawDebugStructure(const FBlockStructure& structure, const FColor& color) const;

	void DrawDebugBlock(class ABlock *block, const FColor& color) const;
#endif

	void AddNeighbours(class ABlock *block, const int& structureIndex, TArray<AActor*>& outArray);

	TArray<class ABlock*> GetNeighbours(class ABlock *block) const;

	// Merge structureB into structureA. Returns structureA and destroys structureB.
	void MergeIntoA(FStructureMeta *metaA, FStructureMeta *metaB);

private:
	TArray<int> m_StructureFreeIndex;

	TArray<FPhysicsBlock> m_PhysicsBlocks;

	UPROPERTY()
	TArray<FBlockStructure> m_Structures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true", DisplayName = "Drop Mode"))
	EDropMode m_DropMode;

public:
	bool m_bDebugRenderStructure;
};
