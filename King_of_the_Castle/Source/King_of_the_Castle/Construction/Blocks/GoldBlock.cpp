// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GoldBlock.h"

#define GOLD_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_GoldBlock.M_GoldBlock'")

AGoldBlock::AGoldBlock()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(GOLD_MATERIAL_LOCATION);

	if (Material.Succeeded())
	{
		Super::m_Mesh->SetMaterial(0, Material.Object);
	}

	Super::m_PointValue = 10;
	Super::m_MaxHealth = Super::m_Health = 100.0f;
}