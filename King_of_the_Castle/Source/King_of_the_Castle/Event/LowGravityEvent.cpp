// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "LowGravityEvent.h"

#include "Runtime/Engine/Classes/GameFramework/WorldSettings.h"

#define GRAVITY_MULTIPLIER 0.5f

ALowGravityEvent::ALowGravityEvent() : m_GravityCache(0.0f)
{
}

void ALowGravityEvent::Start()
{
	Super::Start();

	AWorldSettings *settings = Super::GetWorldSettings();
	checkf(settings != nullptr, TEXT("This level has no world settings?"));
	this->m_GravityCache = settings->GetGravityZ();
	settings->bWorldGravitySet = false;
	settings->bGlobalGravitySet = true;
	settings->GlobalGravityZ = this->m_GravityCache * GRAVITY_MULTIPLIER;
}

void ALowGravityEvent::Stop()
{
	Super::Stop();

	AWorldSettings *settings = Super::GetWorldSettings();
	checkf(settings != nullptr, TEXT("This level has no world settings?"));
	settings->bWorldGravitySet = false;
	settings->GlobalGravityZ = this->m_GravityCache;
}
