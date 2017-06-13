// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Construction/Block.h"
#include "BasicBlock.generated.h"

#define ID_BASIC_BLOCK FName("BasicBlock")

UCLASS()
class KING_OF_THE_CASTLE_API ABasicBlock : public ABlock
{
	GENERATED_BODY()

public:
	ABasicBlock();

	virtual void BeginPlay() override;

	virtual void SetTeam(const int& team) override;

	virtual FName GetNameId() override { return ID_BASIC_BLOCK; }
};
