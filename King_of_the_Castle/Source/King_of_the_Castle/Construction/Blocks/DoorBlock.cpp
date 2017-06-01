// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "DoorBlock.h"

#define DOOR_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_DoorBlock.M_DoorBlock'")

ADoorBlock::ADoorBlock()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(DOOR_MATERIAL_LOCATION);

	if (Material.Succeeded())
	{
		Super::m_Mesh->SetMaterial(0, Material.Object);
	}
	//Super::m_Mesh->SetCollisionProfileName(TEXT("OverlapAll"));

	Super::m_PointValue = 0;
	Super::m_MaxHealth = Super::m_Health = 50.0f;
}

void ADoorBlock::SetTeam(const int& team)
{
	Super::SetTeam(team);

	Super::m_Mesh->SetCollisionProfileName(team == 1 ? TEXT("Team1Block")
		: team == 2 ? TEXT("Team2Block") : TEXT("BlockAllDynamic"));
}
