#pragma once

#include "GameFramework/Actor.h"
#include "Construction/BlockStructureManager.h"

#include "Block.generated.h"

#define BLOCK_DEFAULT_MASS 500.0f //kg
#define BLOCK_DEFAULT_HEALTH 100.0f
#define BLOCK_DEFAULT_SCALE 1.0f //74.660522f
#define BRUSH_SPAWN_Z_OFFSET 0.0f//16.0f // How far up from desired location to spawn a block when placed (so it falls into place)

#define BLOCK_DEFAULT_MESH_LOCATION TEXT("StaticMesh'/Game/Meshes/M_Block.M_Block'")

UCLASS()
class KING_OF_THE_CASTLE_API ABlock : public AActor
{
	GENERATED_BODY()

public:
	ABlock();

	virtual void BeginPlay() override;

	// Create a block entity for this block and then call DestroyBlock on ourself
	virtual TArray<class ABlockEntity*> DropBlock(AActor *source, const bool& restrictPickup = true);

	// Destroy this block (remove it from game)
	virtual void DestroyBlock(AActor *source = nullptr);

	// Set the blocks health
	virtual void SetHealth(const float& health);

	// Whether or not this block can be destroyed
	virtual bool IsDestructable() const { return true; }

	// The id of this block
	virtual FName GetNameId() { return FName("Undefined"); }

	// Get the material of this block. If the material is not dynamic, make it.
	virtual UMaterialInstanceDynamic* GetDynamicMaterial() const;
	
	FORCEINLINE const float& GetHealth() const { return this->m_Health; }

	FORCEINLINE const float& GetMaxHealth() const { return this->m_MaxHealth; }

	FORCEINLINE void SetMaxHealth(const float& health) { this->m_MaxHealth = health; this->m_Health = health; }

	FORCEINLINE const int& GetTeam() const { return this->m_Team; }

	FORCEINLINE virtual void SetTeam(const int& team) { this->m_Team = team; }

	FORCEINLINE const int& GetPointValue() const { return this->m_PointValue; }

	FORCEINLINE UStaticMeshComponent* GetMesh() const { return this->m_Mesh; }

	FORCEINLINE void SetRecipe(TArray<ABlock*> recipe) { this->m_Recipe = recipe; }

	FORCEINLINE TArray<ABlock*>& GetRecipe() { return this->m_Recipe; }

	FORCEINLINE FStructureMeta& GetStructureMeta() { return this->m_StructureMeta; }

	UFUNCTION()
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, class AActor* DamageCauser) override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

	static ABlock* SpawnBlock(UWorld *world, TSubclassOf<ABlock> type, const int& team, AActor *source = nullptr,
		const FVector& location = FVector(0.0f), const FRotator& rotation = FRotator(0.0f));

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (DisplayName = "Mesh"))
	UStaticMeshComponent *m_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block", meta = (DisplayName = "Point Value"))
	int m_PointValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Health"))
	float m_Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Health"))
	float m_MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Min Health Color"))
	FLinearColor m_BlockMinHealthColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Health Color"))
	FLinearColor m_BlockMaxHealthColor;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int m_Team;

	bool m_bLockXY;

	FVector2D m_LockXYPos;

	// Blocks used to make this block
	TArray<ABlock*> m_Recipe;

	// Structure Meta
	FStructureMeta m_StructureMeta;

	/////////////////////
	// Manually break a block using the editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true", DisplayName = "Break Block"))
	bool m_bDebugBreakBlock;
	/////////////////////
};
