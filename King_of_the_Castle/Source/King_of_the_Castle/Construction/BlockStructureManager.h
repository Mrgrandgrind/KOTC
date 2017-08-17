// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BlockStructureManager.generated.h"

UENUM()
enum class EDropMode : uint8
{
	Floating,
	EntityInsta,
	EntityDelay
};

struct FPhysicsBlock
{
	class ABlock *block;
	float counter;
};

struct FBlockPath
{
	bool valid;
	TArray<ABlock*> path, searched;
};

struct FStructureMeta
{
	int index = -1;
	bool isSupport = false;
	bool isDestroyed = false;
};

USTRUCT()
struct FBlockStructure
{
	GENERATED_BODY()

	// The location of the last support to be destroyed.
	// Also set when a block causes one structures to split into two or more.
	class ABlock *lastSupport = nullptr;

	// An array containing all the blocks in the structure
	UPROPERTY()
	TArray<class ABlock*> blocks;

	FBlockStructure() { }

	FBlockStructure(const FBlockStructure& copy) : lastSupport(copy.lastSupport), blocks(copy.blocks)
	{
	}

	FBlockStructure(FBlockStructure&& copy) noexcept : lastSupport(std::move(copy.lastSupport)), blocks(std::move(copy.blocks))
	{
	}

	FBlockStructure& operator=(const FBlockStructure& other)
	{
		this->lastSupport = other.lastSupport;
		this->blocks = other.blocks;
		return *this;
	}

	FBlockStructure& operator=(FBlockStructure&& other) noexcept
	{
		this->lastSupport = std::move(other.lastSupport);
		this->blocks = std::move(other.blocks);
		return *this;
	}
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

	bool IsSupport(class ABlock *block) const;

	bool IsSupport(const FVector& position, const FVector& extent) const;

	TArray<class ABlock*> GetNeighbours(class ABlock *block) const;

	TArray<class ABlock*> GetNeighbours(const FVector& location, FVector extent) const;

	void ProcessDestroy(class ABlock *block);

	void ProcessCreate(class ABlock *block);

	FORCEINLINE const TArray<FBlockStructure>& GetStructures() const { return this->m_Structures; }

	FORCEINLINE FBlockStructure& GetStructure(const int& index) { return this->m_Structures[index]; }

	FORCEINLINE int AddStructure(const FBlockStructure& structure)
	{
		int index = this->FindFreeStructureIndex();
		if(index != -1)
		{
			this->m_Structures[index] = std::move(structure);
			return index;
		}
		return this->m_Structures.Add(structure);
	}

	FORCEINLINE int AddStructure(FBlockStructure&& structure)
	{
		int index = this->FindFreeStructureIndex();
		if (index != -1)
		{
			this->m_Structures[index] = std::move(structure);
			return index;
		}
		return this->m_Structures.Add(FBlockStructure(structure));
	}

protected:
	FORCEINLINE int FindFreeStructureIndex()
	{
		return this->m_StructureFreeIndex.Num() > 0 ? this->m_StructureFreeIndex.Pop() : -1;
	}

	void DropBlock(class ABlock *block);

	bool CheckStructureSupport(FBlockStructure& structure);

	FBlockPath GeneratePath(class ABlock *from, class ABlock *to, class ABlock *ignored = nullptr) const;

	void ProcessPreplaced();

#if WITH_EDITOR
	void DrawDebugStructure(FStructureMeta *meta, const FColor& color) const;

	void DrawDebugStructure(const FBlockStructure& structure, const FColor& color) const;

	void DrawDebugBlock(class ABlock *block, const FColor& color) const;
#endif

	void AddNeighbours(class ABlock *block, const int& structureIndex, TArray<AActor*>& outArray);

	// Merge structureB into structureA. Returns structureA and destroys structureB.
	void MergeIntoA(FStructureMeta *metaA, FStructureMeta *metaB);

private:
	TArray<int> m_StructureFreeIndex;

	TArray<FPhysicsBlock> m_PhysicsBlocks;

	UPROPERTY()
	TArray<FBlockStructure> m_Structures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true", DisplayName = "Drop Mode"))
	EDropMode m_DropMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true", DisplayName = "Support: Wall"))
	bool m_bSupportWall;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true", DisplayName = "Support: Wall Top"))
	//bool m_bSupportWallTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true", DisplayName = "Support: Ground"))
	bool m_bSupportGround;

public:
	bool m_bDebugRenderStructure;
};
