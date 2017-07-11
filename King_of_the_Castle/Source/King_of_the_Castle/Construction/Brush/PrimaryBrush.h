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

	virtual class ABlock* Action(class ABuildArea *area, AActor *source) override;

	virtual void Update(class APlayerCharacter *character, class ABuildArea *area, const FHitResult& trace) override;

	void SetSelectedIndex(int index);

	UBlockData* GetBlockData(const int& index);

	int GetIndexOf(const FName id) const;

	FORCEINLINE int GetBlockDataCount() const { return this->m_BlockData.Num(); }

	FORCEINLINE const int& GetSelectedIndex() const { return this->m_SelectedTypeIndex; }

	FORCEINLINE UBlockData* GetBlockData(const FName id) { return this->GetBlockData(this->GetIndexOf(id)); }

	FORCEINLINE void SetPrefab(TSubclassOf<class APrefab> prefab) { this->m_Prefab = prefab; this->UpdateBlockChild(); }

protected:
	void UpdateBlockChild();

	void UpdateChain(class ABuildArea *area, const FHitResult& trace, bool& show);

	void UpdateRegular(class ABuildArea *area, const FHitResult& trace, bool& show);

private:
	bool m_bValid;

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

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true", DisplayName = "Render Trace"))
	bool m_bDebugRenderTrace;
};
