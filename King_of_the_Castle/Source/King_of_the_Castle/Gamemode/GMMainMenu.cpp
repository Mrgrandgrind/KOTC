// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMMainMenu.h"

#include "SplineTrack.h"
#include "Character/DefaultPlayerCharacter.h"

#include "Runtime/Engine/Classes/Components/SplineComponent.h"

#define DEFAULT_TRACK_DURATION 40.0f
#define CAMERA_PIVOT_POINT FVector(13.0f, -3389.0f, 700.0f)

AGMMainMenu::AGMMainMenu() : m_Duration(DEFAULT_TRACK_DURATION), m_StartTime(0.0f)
{
	Super::DefaultPawnClass = ADefaultPlayerCharacter::StaticClass();
}

void AGMMainMenu::BeginPlay()
{
	Super::BeginPlay();

	if (Super::GetWorld() == nullptr)
	{
		return;
	}

	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ASplineTrack::StaticClass(), out);

	if (out.Num() == 0)
	{
		UE_LOG(LogClass, Log, TEXT("[GMMainMenu] No ASplineTrack in map!"));
	}
	else 
	{
		if (out.Num() > 1)
		{
			UE_LOG(LogClass, Log, TEXT("[GMMainMenu] Too many ASplineTrack's in map! Expected 1, found %d"), out.Num());
		}
		this->m_Track = Cast<ASplineTrack>(out[0]);
		this->m_Track->GetSpline()->Duration = this->m_Duration;
	}
}

APawn* AGMMainMenu::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot)
{
	APawn *pawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	if (pawn != nullptr)
	{
		this->m_Character = Cast<ADefaultPlayerCharacter>(pawn);
	}
	return pawn;
}

void AGMMainMenu::Tick(float delta)
{
	Super::Tick(delta);

	if (this->m_Character == nullptr || this->m_Track == nullptr)
	{
		return;
	}
	this->m_Counter += delta;

	USplineComponent *spline = this->m_Track->GetSpline();
	if (this->m_Counter > spline->Duration)
	{
		this->m_Counter -= spline->Duration;
	}

	FVector vector = spline->GetLocationAtTime(this->m_Counter, ESplineCoordinateSpace::World, false);
	this->m_Character->SetActorLocation(vector);
	this->m_Character->GetCamera()->SetWorldRotation((CAMERA_PIVOT_POINT - vector).Rotation());
}

#if WITH_EDITOR
void AGMMainMenu::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(AGMMainMenu, m_Duration) && this->m_Track != nullptr)
	{
		this->m_Track->GetSpline()->Duration = this->m_Duration;
	}
}
#endif