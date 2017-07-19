#pragma once

#include "Construction/Brush/BlockBrush.h"
#include "PrimaryBrush.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API UPrimaryBrush : public UBlockBrush
{
	GENERATED_BODY()

public:
	UPrimaryBrush();

	virtual void BeginPlay() override;

	// Drop x amount of blocks from primary brush
	virtual void DropBlocks(class UBlockData *data, int count);

	virtual void SetBrushVisible(const bool& visible) override;

	void SetSelectedIndex(int index);

	UBlockData* GetBlockData(const int& index);

	int GetIndexOf(const FName id) const;

	virtual void OnTeamChange(const int& team) override { this->UpdateChildBlocks(); }

	FORCEINLINE int GetBlockDataCount() const { return this->m_BlockData.Num(); }

	FORCEINLINE const int& GetSelectedIndex() const { return this->m_SelectedTypeIndex; }

	FORCEINLINE UBlockData* GetBlockData(const FName id) { return this->GetBlockData(this->GetIndexOf(id)); }

	FORCEINLINE void SetPrefab(TSubclassOf<class APrefab> prefab) { this->m_Prefab = prefab; /*this->UpdateBlockChildActor();*/ }

protected:
	FRotator GetBrushRotation() const;


	bool CanPlaceOn(const FHitResult& result) const;

	void UpdateChildBlocks();

	virtual TArray<class ABlock*> OnAction(class ABuildArea *area, AActor *source) override;

	virtual bool OnPreCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show) override;

	virtual bool OnMainCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& pre) override;

	virtual bool OnPostCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& valid) override;

private:
	FRotator m_Rotation;

	UPROPERTY()
	TArray<ABlock*> m_ChildBlocks;

	UPROPERTY()
	TSubclassOf<class APrefab> m_Prefab;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Type", meta = (AllowPrivateAccess = "true", DisplayName = "Selected Index"))
	int m_SelectedTypeIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Type", meta = (AllowPrivateAccess = "true", DisplayName = "Block Data"))
	TArray<UBlockData*> m_BlockData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (AllowPrivateAccess = "true", DisplayName = "Data Classes"))
	TArray<TSubclassOf<UBlockData>> m_BlockDataClasses;
};
