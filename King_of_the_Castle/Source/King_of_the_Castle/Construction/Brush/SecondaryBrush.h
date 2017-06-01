#pragma once

#include "Construction/Brush/BlockBrush.h"
#include "SecondaryBrush.generated.h"

UENUM(BlueprintType)
enum class EModifyMode : uint8
{
	Undefined, Destroy, Combine
};

USTRUCT()
struct FBlockRecipe
{
	GENERATED_BODY()

	// How long the combination process should take
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine", meta = (DisplayName = "Duration"))
	float m_Duration;

	// One type of block we need for combining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine", meta = (DisplayName = "TypeA"))
	FName m_TypeA;

	// One type of block we need for combining
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine", meta = (DisplayName = "TypeB"))
	FName m_TypeB;

	// What the combination of blocks produces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine", meta = (DisplayName = "Result Type"))
	TSubclassOf<class ABlock> m_ResultClassType;
};

UCLASS()
class KING_OF_THE_CASTLE_API USecondaryBrush : public UBlockBrush
{
	GENERATED_BODY()
	
public:
	USecondaryBrush();

	// Set the mode of the modify brush
	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetMode(const EModifyMode& mode);

	// Preform the destroy action. Combining is done within Update.
	UFUNCTION(BlueprintCallable, Category = "Brush")
	virtual class ABlock* Action(class ABuildArea *area, AActor *source) override;

	// Update chaining
	void UpdateChain(class ABuildArea *area);

	// Update crafting
	void UpdateCrafting(class ABlock *block, class UPrimaryBrush *primary, class UBlockData *activeData);

	// Update the brush
	virtual void Update(class APlayerCharacter *character, class ABuildArea *area, const FHitResult& trace) override;

	// Give the modify brush an instance of the timer used for combining blocks. This must be done on initialization.
	FORCEINLINE void SetCraftTimer(float *timer) { this->m_CraftTimer = timer; }

	// Get current mode of the modify brush (Combine or Destroy)
	FORCEINLINE const EModifyMode& GetMode() const { return this->m_Mode; }

	// Whether or not the brush is currently trying to combine two blocks
	FORCEINLINE bool IsCombining() const { return this->m_bCombining || this->m_CombineLock != nullptr; }

	// Combine lock must be released whenever the player is no longer trying to combine blocks (released key)
	FORCEINLINE void ReleaseCombineLock() { this->m_bCombining = false; this->m_CombineLock = nullptr; }

private:
	// The current mode of the brush
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mode", meta = (AllowPrivateAccess = "true", DisplayName = "Mode"))
	EModifyMode m_Mode;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combine", meta = (AllowPrivateAccess = "true", DisplayName = "Combine Box"))
	UStaticMeshComponent *m_CombineBox;

	// A list of all recipes that this brush can make
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combine", meta = (AllowPrivateAccess = "true", DisplayName = "Recipes"))
	TArray<FBlockRecipe> m_Recipes;

	// Whether or not the brush is combining two blocks right now
	bool m_bCombining;

	// If combining, this variable contains the target block that we are combining with
	ABlock *m_CombineLock;

	// The timer used for combining
	float *m_CraftTimer;

	// The current recipe we are doing. Chosen from the array of recipes above.
	const FBlockRecipe *m_ActiveRecipe;
};
