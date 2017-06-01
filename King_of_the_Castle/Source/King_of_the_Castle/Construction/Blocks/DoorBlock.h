// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Construction/Block.h"
#include "DoorBlock.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API ADoorBlock : public ABlock
{
	GENERATED_BODY()
	
public:
	ADoorBlock();

	virtual void SetTeam(const int& team) override;

	virtual FName GetNameId() override { return FName("DoorBlock"); }
};
