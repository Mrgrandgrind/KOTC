// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Construction/Block.h"
#include "BlockEntity.generated.h"

#define ID_BLOCK_ENTITY FName("BlockEntity")

UCLASS()
class KING_OF_THE_CASTLE_API ABlockEntity : public ABlock
{
	GENERATED_BODY()
	
public:
	ABlockEntity();

	virtual void BeginPlay() override;

	virtual void Tick(float delta) override;
	
	// Entity blocks cannot be destroyed
	virtual bool IsDestructable() const override { return false; }

	virtual FName GetNameId() override { return ID_BLOCK_ENTITY; }

	// Set this entity as another block
	void SetTo(ABlock *block);

	// Returns whether or not the given player is allowed to pick up this block
	bool CanBePickedUp(class APlayerCharacter *character) const;

	// Pickup this block. This block will be destroyed and the players create brush will gain 1 of this block.
	void Pickup(class APlayerCharacter *character);

	// Force this entity to despawn. This is not an instant process because of the animation.
	void ForceDespawn();

	// Skip the spawn animation
	void SkipSpawnAnimation();

	// Set duration that this entity will exist
	void SetLifeTime(const float& time);

	FORCEINLINE AActor* GetBlockOwner() const { return this->m_Owner; }

	FORCEINLINE void SetBlockOwner(AActor *actor) { this->m_Owner = actor; }

	FORCEINLINE void SetRestrictedPickup(const bool& restrict) { this->m_bRestrictedPickup = restrict; }

	FORCEINLINE void SetIgnoreOwner(const bool& ignore) { this->m_bIgnoreOwner = ignore; }

	FORCEINLINE const FName& GetParentBlockNameId() const { return this->m_ParentBlockNameId; }

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	static TArray<ABlockEntity*> SpawnBlockEntity(ABlock *block, UWorld *world, AActor *source, const bool& restrictPickup = true);

	static TArray<ABlockEntity*> SpawnBlockEntity(ABlock *block, AActor *source, const bool& restrictPickup = true);

private:
	// Time since created and time to exist for
	float m_CreateCounter, m_LifeTime;

	// Who is responsible for this block spawning
	UPROPERTY()
	AActor *m_Owner;

	FVector m_BaseScale, m_DesiredScale;

	// Whether or not to ignore the owner for pickup up or only go to the owner
	bool m_bIgnoreOwner;

	// Whether or not this block can be picked up regardless of the players current count of this block
	bool m_bRestrictedPickup;

	// What this block represents
	UPROPERTY()
	FName m_ParentBlockNameId;

	// All overlapping players
	UPROPERTY()
	TArray<class APlayerCharacter*> m_Collisions;
};
