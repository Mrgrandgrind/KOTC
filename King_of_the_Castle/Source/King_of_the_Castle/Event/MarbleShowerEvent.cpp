// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "MarbleShowerEvent.h"

#include "DrawDebugHelpers.h"

#define RAND_RADIUS (-this->m_Radius / 2.0f + this->m_Radius * FMath::FRand())

AMarbleShowerEvent::AMarbleShowerEvent()
{
	this->m_Counter = 0.0f;
	this->m_Radius = 8000.0f;
	this->m_SpawnDelay = 0.05f;
	this->m_RayHeight = 2500.0f;

	Super::m_Duration = 30.0f;
	Super::PrimaryActorTick.bCanEverTick = true;
}

void AMarbleShowerEvent::Tick(float delta)
{
	Super::Tick(delta);

	if(this->m_MarbleClass == nullptr)
	{
		return;
	}

	this->m_Counter += delta;
	if(this->m_Counter > this->m_SpawnDelay)
	{
		FVector start = this->m_OriginPosition + FVector(RAND_RADIUS, RAND_RADIUS, 0.0f);
		FVector end = start + FVector(0.0f, 0.0f, this->m_RayHeight);

		FHitResult result;
		Super::GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_WorldStatic);

		//DrawDebugLine(Super::GetWorld(), start, end, FColor::Red, false, 5.0f, 0, 4.0f);
		//DrawDebugPoint(Super::GetWorld(), start, 10.0f, FColor::Purple, false, 2.0f, 0);

		if(result.GetComponent() != nullptr)
		{
			//DrawDebugPoint(Super::GetWorld(), result.ImpactPoint, 10.0f, FColor::Orange, false, 2.0f, 0);

			FVector normal = (end - start).GetSafeNormal();
			Super::GetWorld()->SpawnActor<AActor>(this->m_MarbleClass, result.ImpactPoint - normal * 100.0f, FRotator(0.0f));

			this->m_Counter -= this->m_SpawnDelay;
		}
	}
}

void AMarbleShowerEvent::Start()
{
}

void AMarbleShowerEvent::Stop()
{
}
