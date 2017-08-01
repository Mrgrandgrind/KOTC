// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "DefaultPlayerCharacter.h"

ADefaultPlayerCharacter::ADefaultPlayerCharacter()
{
	this->m_Camera = UObject::CreateDefaultSubobject<UCameraComponent>(TEXT("DefaultCamera"));
	this->m_Camera->SetupAttachment(Super::RootComponent);

	((UPrimitiveComponent*)Super::GetRootComponent())->SetSimulatePhysics(false);
}