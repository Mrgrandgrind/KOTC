// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "FloorIsLavaEvent.h"

#include "../Character/PlayerCharacter.h"

#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"

#define LAVA_DAMAGE 1.5f //per second

#define LAVA_FADE_DURATION 1.5f

#define LAVA_MATERIAL_ALPHA_NAME TEXT("Alpha")
#define LAVA_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_Lava.M_Lava'")

#define TRIGGER_Z_MULTIPLIER 20.0f

AFloorIsLavaEvent::AFloorIsLavaEvent() : m_Floor(nullptr), m_LastAlpha(0.0f)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(LAVA_MATERIAL_LOCATION);

	this->m_Trigger = UObject::CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDetector"));
	Super::RootComponent = this->m_Trigger;

	FScriptDelegate delegateBegin, delegateEnd;
	delegateBegin.BindUFunction(this, "BeginOverlap");
	this->m_Trigger->OnComponentBeginOverlap.Add(delegateBegin);
	delegateEnd.BindUFunction(this, "EndOverlap");
	this->m_Trigger->OnComponentEndOverlap.Add(delegateEnd);

	if (Material.Succeeded())
	{
		this->m_Lava = Material.Object;
	}

	Super::m_Duration = DEFAULT_EVENT_DURATION;
	Super::m_FadeInDuration = LAVA_FADE_DURATION;
	Super::m_FadeOutDuration = LAVA_FADE_DURATION;
	Super::PrimaryActorTick.bCanEverTick = true;
}

void AFloorIsLavaEvent::Tick(float delta)
{
	Super::Tick(delta);
	if (!Super::IsRunning() || this->m_Floor == nullptr || this->m_Lava == nullptr)
	{
		return;
	}
	FDamageEvent damageEvent;
	for (APlayerCharacter *character : this->m_Overlapping)
	{
		FVector origin, extent;
		character->GetActorBounds(true, origin, extent);

		FVector pointA = origin - FVector(0.0f, 0.0f, extent.Z * 0.75f);
		FVector pointB = origin - FVector(0.0f, 0.0f, extent.Z * 1.25f);

		FCollisionQueryParams params;
		params.AddIgnoredActor(character);

		FHitResult result;
		Super::GetWorld()->LineTraceSingleByChannel(result, pointA, pointB, ECollisionChannel::ECC_WorldDynamic, params);
		//DrawDebugLine(Super::GetWorld(), pointA, pointB, FColor::Red, false, -1.0f, 0, 4.0f);

		if (result.GetActor() == this->m_Floor)
		{
			character->TakeDamage(LAVA_DAMAGE * delta, damageEvent, character->GetController(), this->m_Floor);
		}
	}
	if (Super::IsFadeActive() || this->m_LastAlpha != 0.0f || this->m_LastAlpha != 1.0f)
	{
		UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_Floor->GetStaticMeshComponent()->GetMaterial(0));
		if (material == nullptr)
		{
			material = UMaterialInstanceDynamic::Create(this->m_Lava, this->m_Floor->GetStaticMeshComponent());
			this->m_Floor->GetStaticMeshComponent()->SetMaterial(0, material);
		}
		material->SetScalarParameterValue(LAVA_MATERIAL_ALPHA_NAME, this->m_LastAlpha = Super::GetFadePercentage());
	}
}

void AFloorIsLavaEvent::Start()
{
	Super::Start();

	if (this->m_Lava == nullptr)
	{
		return;
	}
	for (TActorIterator<AStaticMeshActor> itr(Super::GetWorld()); itr; ++itr)
	{
		if (itr->GetName() == TEXT("Floor"))
		{
			this->m_Floor = *itr;

			FVector origin, extent;
			this->m_Floor->GetActorBounds(true, origin, extent);
			extent.Z *= TRIGGER_Z_MULTIPLIER;
			this->m_Trigger->SetWorldLocation(origin);
			this->m_Trigger->SetBoxExtent(extent, true);

			UStaticMeshComponent *mesh = this->m_Floor->GetStaticMeshComponent();
			this->m_PreviousMaterial = mesh->GetMaterial(0);
			mesh->SetMaterial(0, this->m_Lava);

			break;
		}
	}
}

void AFloorIsLavaEvent::Stop()
{
	Super::Stop();

	if (this->m_Floor == nullptr)
	{
		return;
	}
	this->m_Floor->GetStaticMeshComponent()->SetMaterial(0, this->m_PreviousMaterial);
	this->m_Floor = nullptr;
	this->m_PreviousMaterial = nullptr;
	this->m_Overlapping.Empty();
}

void AFloorIsLavaEvent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	if (character == nullptr)
	{
		return;
	}
	this->m_Overlapping.Add(character);
}

void AFloorIsLavaEvent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	if (character == nullptr)
	{
		return;
	}
	this->m_Overlapping.Remove(character);
}