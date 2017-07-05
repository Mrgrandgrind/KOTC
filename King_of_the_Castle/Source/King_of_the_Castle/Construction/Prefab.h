// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Prefab.generated.h"

struct FPrefabBlock
{
	FName id;
	FIntVector offset;
};

UCLASS()
class KING_OF_THE_CASTLE_API APrefab : public AActor
{
	GENERATED_BODY()
	
public:	
	APrefab();

	virtual void BeginPlay() override;

	FVector GetPerfabLocation(const FIntVector& offset) const;

	FORCEINLINE const TArray<FPrefabBlock>& GetBlocks() { if (this->m_Blocks.Num() == 0) this->PopulateBlocks(); return this->m_Blocks; }

//#if WITH_EDITOR
//	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
//#endif

	void PopulateBlocks();
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prefab", meta = (AllowPrivateAccess = "true", DisplayName = "Grid Size"))
	FVector m_GridSize;

	// Blocks contained within this prefab
	TArray<FPrefabBlock> m_Blocks;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true", DisplayName = "Snap Positions (Broke: Doesn't save)"))
	//bool m_bDebugSnapPositions;
};
