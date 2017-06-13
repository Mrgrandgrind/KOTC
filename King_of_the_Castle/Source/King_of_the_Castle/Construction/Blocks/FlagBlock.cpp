// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "FlagBlock.h"

#include "Character/PlayerCharacter.h"

#define HIT_REVIVE_TIME 2.5f //seconds

#define DEFAULT_HITS_TO_BREAK 4

AFlagBlock::AFlagBlock() : m_Hits(0), m_HitsToBreak(DEFAULT_HITS_TO_BREAK), m_DamageCounter(HIT_REVIVE_TIME), m_ReviveCounter(0.0f)
{
	Super::PrimaryActorTick.bCanEverTick = true;
}

void AFlagBlock::DropBlock(AActor* source, const bool& restrictPickup)
{
	// Flags must always have restricted pickup. 
	// The player should never be able to hold more than one at a given time.
	Super::DropBlock(source, true);
}

void AFlagBlock::Tick(float delta)
{
	Super::Tick(delta);

	if(this->m_DamageCounter < HIT_REVIVE_TIME)
	{
		this->m_DamageCounter += delta;
		return;
	}
	if(this->m_Hits <= 0)
	{
		return;
	}
	this->m_ReviveCounter += delta;
	if (this->m_ReviveCounter > HIT_REVIVE_TIME)
	{
		if(--this->m_Hits <= 0)
		{
			this->m_ReviveCounter = 0.0f;
		}
		else
		{
			this->m_ReviveCounter -= HIT_REVIVE_TIME;
		}
		Super::SetHealth(Super::m_MaxHealth * (1.0f - this->m_Hits / float(this->m_HitsToBreak)));
	}
}

float AFlagBlock::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	APlayerCharacter *character = Cast<APlayerCharacter>(DamageCauser);
	if(character == nullptr || character->GetTeam() == Super::GetTeam())
	{
		return 0.0f;
	}
	this->m_Hits++;
	this->m_DamageCounter = 0.0f;

	Super::SetHealth(Super::m_MaxHealth * (1.0f - this->m_Hits / float(this->m_HitsToBreak)));
	if(this->m_Hits >= this->m_HitsToBreak)
	{
		Super::DropBlock(DamageCauser, true);
	}
	return 0.0f;
}
