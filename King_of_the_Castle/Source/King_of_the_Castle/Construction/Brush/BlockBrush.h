// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

#include "BlockBrush.generated.h"

#define BRUSH_COLOR_NAME TEXT("Color")

#define CELLS_EQUAL(a, b) (a.X == b.X && a.Y == b.Y && a.Z == b.Z)
#define CELLS_BESIDE(a, b) (FMath::Abs(a.X - b.X) + FMath::Abs(a.Y - b.Y) + FMath::Abs(a.Z - b.Z) == 1)
#define CELLS_IN_DIRECTION(a, b, d) CELLS_EQUAL((a - b), d)

typedef FIntVector FGridCell;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBrushPreCheckSignature, ABuildArea*, area, const FHitResult&, result, bool&, valid);

UCLASS()
class KING_OF_THE_CASTLE_API UBlockBrush : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UBlockBrush();

	virtual void BeginPlay() override;

	virtual void OnTeamChange(const int& team) { }

	void SetChainMode(const bool& enable);

	TArray<class ABlock*> Action(class ABuildArea *area, AActor *source);

	virtual void Update(class APlayerCharacter *character, class ABuildArea *area, const FHitResult& trace);

	virtual void UpdateCountText(class UBlockData *data = nullptr, const FVector *ownerLocation = nullptr);

	virtual void SetBrushVisible(const bool& visible) { Super::SetHiddenInGame(!visible, true); }

	FORCEINLINE void SetTeam(int *team) { this->m_Team = team; }

	FORCEINLINE bool IsBrushVisible() const { return !Super::bHiddenInGame; }

	FORCEINLINE const FIntVector& GetActiveCell() const { return this->m_ActiveCell; }

	FORCEINLINE bool IsChaining() const { return this->m_Chained.Num() > 1; }

protected:
	UMaterialInstanceDynamic* GetMaterialDynamic();

	bool IsSupport(const FVector& position, const FVector& cellSize) const;

	virtual bool IsOverlapped() const;

	virtual TArray<class ABlock*> OnAction(class ABuildArea *area, AActor *source) { return TArray<class ABlock*>(); }

	virtual bool OnPreCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show) { return false; }

	virtual bool OnMainCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& pre) { return false; }

	virtual bool OnPostCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& valid) { return valid; }

	FORCEINLINE bool IsPositionValid() const { return this->m_bPositionValid; }

	FORCEINLINE void RenderPoint(const FVector& point, const FColor& color) const
	{
#if WITH_EDITOR
		if (this->m_bDebugRenderTrace)
		{
			DrawDebugPoint(Super::GetWorld(), point, 10.0f, color);
		}
#endif
	}

	FORCEINLINE void RenderTrace(const FVector& start, const FVector& end, const FColor& color) const
	{
#if WITH_EDITOR
		if (this->m_bDebugRenderTrace)
		{
			DrawDebugLine(Super::GetWorld(), start, end, color, false, -1.0f, 0.0f, 2.0f);
		}
#endif
	}

	int *m_Team;

	bool m_bChainMode;

	// Whether or not a block can be placed at the current location.
	// An additional check should be conducted before placing the actual block
	// because this data could potentially be outdated by the time the player uses it
	bool m_bPositionValid;

	// The current cell location
	FGridCell m_ActiveCell;

	FHitResult m_LastTrace;

	FRotator m_ChainedRotation;

	TArray<FIntVector> m_Chained;

	UPROPERTY()
	class ATextRenderActor *m_TextActor;

	UPROPERTY()
	UMaterialInterface *m_Material;

public:
//// Debug ////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true", DisplayName = "Render Trace"))
	bool m_bDebugRenderTrace;
///////////////
};
