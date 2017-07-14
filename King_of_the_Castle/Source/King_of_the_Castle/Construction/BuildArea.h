// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BuildArea.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ABuildArea : public AActor
{
	GENERATED_BODY()
	
public:
	ABuildArea();

	virtual void BeginPlay() override;

	virtual void Tick(float delta) override;

	// Calculate the grid size from the cell size. Should be called whenever the cell size is updated.
	void CalculateGridSize();

	// Return whether or not this cell lies within the grid boundaries (it's a basic box bounds check)
	bool IsCellValid(const FIntVector& cell) const;

	// Get the grid cell from the provided world location. Puts the cell value into the provided IntVector.
	// Returns whether or not the given cell is valid (See IsCellValid())
	bool GetGridCell(const FVector& location, FIntVector& out) const;

	// Get the world location from the provided cell value. Puts the world location into the provided Vector.
	// Returns whether or not the cell is valid (See IsCellValid() - If not valid, out value is not set)
	bool GetGridLocation(const FIntVector& cell, FVector& out) const;

	// Spawn a block of the given type (must derive from ABlock) at the given cell.
	// Returns the actor that was spawned, or nullptr if the cell is not valid. (See IsCellValid())
	class ABlock* SpawnBlockAt(const FIntVector& cell, TSubclassOf<class ABlock> blockClass, class APlayerCharacter *source = nullptr) const;

	// Get the individual grid cell size
	FORCEINLINE const FVector& GetCellSize() const { return this->m_CellSize; }

	// Set the individual grid cell size
	FORCEINLINE void SetCellSize(const FVector& vector) { this->m_CellSize.Set(vector.X, vector.Y, vector.Z); }

	// Get the team that this build area is supposed to be for
	FORCEINLINE const int& GetTeam() const { return this->m_Team; }

	// Set the team that this build area is supposed to be for
	FORCEINLINE void SetTeam(const int& team) { this->m_Team = team; }

	FORCEINLINE bool CanTeamBuild(const int& team) { return this->m_bIgnoreTeam || this->m_Team == team; }

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
		UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
		UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

private:
	// The box bounds used to define this area (root component)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Area", meta = (AllowPrivateAccess = "true", DisplayName = "Box Area"))
	UBoxComponent *m_Area;

	// The team that this build area is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int m_Team;

	// Ignore the team and allow anyone to build here
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Ignore Team"))
	bool m_bIgnoreTeam;

	// The size of an individual cell on the grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "Cell Size"))
	FVector m_CellSize;

	// The dimensions of the grid. Should not be manually written to. See CalculateGridSize().
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "Grid Size"))
	FIntVector m_GridSize;

	// DEBUG VARIABLES //
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Set Cell Size From..."))
	AActor *m_DebugCellSizeMesh;

	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Render Grid"))
	bool m_DebugRenderGrid;

	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Force Grid Update"))
	bool m_DebugForceUpdate;

	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Snap Placed Blocks"))
	bool m_DebugSnapPlaced;

	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Constrain Placed Blocks"))
	bool m_DebugConstrain;

	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Set Placed Blocks Team"))
	int m_DebugTeam;
	/////////////////////

};
