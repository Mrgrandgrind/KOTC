// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BasicBlock.h"

#include "GameMode/BaseGameMode.h"

#define MATERIAL_TEAM_NAME TEXT("TeamColor")

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
	ABaseGameMode *gamemode = GetGameMode(Super::GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	UMaterialInstanceDynamic *material = Super::GetDynamicMaterial();
	if (material == nullptr)
	{
		return;
	}
	material->SetVectorParameterValue(MATERIAL_TEAM_NAME, gamemode->GetTeamColor(team));
}
