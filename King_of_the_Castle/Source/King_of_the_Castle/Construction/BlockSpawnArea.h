// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BlockSpawnArea.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ABlockSpawnArea : public AActor
{
	GENERATED_BODY()
	
public:
	ABlockSpawnArea();

	bool TrySpawn(FVector& out) const;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

private:
	bool TrySpawnNextTo(class ABlock *block, FVector& out) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area", meta = (AllowPrivateAccess = "true", DisplayName = "Box Area"))
	UBoxComponent *m_Area;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Area", meta = (AllowPrivateAccess = "true", DisplayName = "Spawn Box"))
	UBoxComponent *m_SpawnBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area", meta = (AllowPrivateAccess = "true", DisplayName = "Spawn Box Extent"))
	FVector m_SpawnBoxExtent;

	// Works like odds. The highest rating has the highest chance of spawning a block.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (AllowPrivateAccess = "true", DisplayName = "Spawn Rating"))
	float m_SpawnRating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Set Spawn"))
	bool m_bDebugSetSpawn;
};
