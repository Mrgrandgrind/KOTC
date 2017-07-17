#pragma once

#include "Construction/Brush/BlockBrush.h"
#include "SecondaryBrush.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API USecondaryBrush : public UBlockBrush
{
	GENERATED_BODY()
	
public:
	USecondaryBrush();

	virtual void BeginPlay() override;

protected:
	virtual TArray<class ABlock*> OnAction(class ABuildArea *area, AActor *source) override;

	virtual bool OnMainCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& pre) override;
};
