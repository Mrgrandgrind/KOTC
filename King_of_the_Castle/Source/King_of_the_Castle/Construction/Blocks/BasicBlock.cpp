// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BasicBlock.h"

#define MATERIAL_TEAM_NAME TEXT("TeamColor")
#define BLOCK_TEAM1_COLOR FLinearColor(0.4f, 0.4f, 1.0f, 1.0f)
#define BLOCK_TEAM2_COLOR FLinearColor(1.0f, 0.3f, 0.3f, 1.0f)

ABasicBlock::ABasicBlock()
{
	Super::m_PointValue = 1;
	Super::m_MaxHealth = Super::m_Health = 100.0f;
}

void ABasicBlock::BeginPlay()
{
	Super::BeginPlay();
	// Update material
	this->SetTeam(Super::GetTeam());
}

void ABasicBlock::SetTeam(const int& team)
{
	Super::SetTeam(team);

	if (team <= 0)
	{
		return;
	}
	UMaterialInstanceDynamic *material = Super::GetDynamicMaterial();
	if (material == nullptr)
	{
		return;
	}
	FLinearColor color;
	switch (Super::GetTeam())
	{
		case 1:
			color = BLOCK_TEAM1_COLOR;
			break;
		case 2:
			color = BLOCK_TEAM2_COLOR;
			break;
		default:
			color = FLinearColor::White;
	}
	material->SetVectorParameterValue(MATERIAL_TEAM_NAME, color);
}
