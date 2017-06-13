// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Construction/Block.h"
#include "GoldBlock.generated.h"

#define ID_GOLD_BLOCK FName("GoldBlock")

UCLASS()
class KING_OF_THE_CASTLE_API AGoldBlock : public ABlock
{
	GENERATED_BODY()
	
public:
	AGoldBlock();

	virtual FName GetNameId() override { return ID_GOLD_BLOCK; }
};
