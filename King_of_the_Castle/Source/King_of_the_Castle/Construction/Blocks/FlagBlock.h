// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GoldBlock.h"
#include "FlagBlock.generated.h"

#define ID_FLAG_BLOCK FName("FlagBlock")

UCLASS()
class KING_OF_THE_CASTLE_API AFlagBlock : public AGoldBlock
{
	GENERATED_BODY()

public:
	AFlagBlock();

	virtual void Tick(float delta) override;

	virtual FName GetNameId() override { return ID_FLAG_BLOCK; }

	virtual TArray<class ABlockEntity*> DropBlock(AActor *source, const bool& restrictPickup) override;

	FORCEINLINE void SetHitsToBreak(const int& num) { this->m_HitsToBreak = num; }

	UFUNCTION()
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, class AActor* DamageCauser) override;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flag", meta = (AllowPrivateAccess = "true", DisplayName = "Hits"))
	int m_Hits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flag", meta = (AllowPrivateAccess = "true", DisplayName = "Hits to Break"))
	int m_HitsToBreak;

	// Time since last damage
	float m_DamageCounter;

	// Counter used to revive the amount of hits that this block has taken
	float m_ReviveCounter;
};
