// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BlockBrush.generated.h"

#define BRUSH_COLOR_NAME TEXT("Color")

#define CELLS_EQUAL(a, b) (a.X == b.X && a.Y == b.Y && a.Z == b.Z)
#define CELLS_BESIDE(a, b) (FMath::Abs(a.X - b.X) + FMath::Abs(a.Y - b.Y) + FMath::Abs(a.Z - b.Z) == 1)
#define CELLS_IN_DIRECTION(a, b, d) CELLS_EQUAL((a - b), d)

UCLASS()
class KING_OF_THE_CASTLE_API UBlockBrush : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UBlockBrush();

	virtual void BeginPlay() override;

	virtual void SetBrushVisible(const bool& visible);

	virtual bool SetPositionToCell(class ABuildArea *area, const FIntVector& cell);

	virtual class ABlock* Action(class ABuildArea *area, AActor *source) { return nullptr; }

	virtual void Update(class APlayerCharacter *character, class ABuildArea *area, const FHitResult& trace);

	virtual void SetChainMode(const bool& enable);

	virtual void UpdateCountText(class UBlockData *data = nullptr, const FVector *ownerLocation = nullptr);

	FORCEINLINE const FIntVector& GetActiveCell() const { return this->m_ActiveCell; }

	FORCEINLINE virtual bool IsBrushVisible() const { return !Super::bHiddenInGame; }

	FORCEINLINE void SetTeam(int *team) { this->m_Team = team; }

	FORCEINLINE virtual bool IsChaining() const { return this->m_Chained.Num() > 1; }

protected:
	virtual bool IsOverlapped() const;

	UMaterialInstanceDynamic* GetMaterialDynamic();

	//The current grid cell that this brush is looking at
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true", DisplayName = "Active Cell"))
	FIntVector m_ActiveCell;

	int *m_Team;

	bool m_bChainMode;

	FHitResult m_LastTrace;

	FRotator m_ChainedRotation;

	TArray<FIntVector> m_Chained;

	UPROPERTY()
	class ATextRenderActor *m_TextActor;

private:
	UPROPERTY()
	UMaterialInterface *m_Material;
};
