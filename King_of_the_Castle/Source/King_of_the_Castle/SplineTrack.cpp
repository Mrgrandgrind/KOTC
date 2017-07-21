// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "SplineTrack.h"

#include "Runtime/Engine/Classes/Components/SplineComponent.h"

ASplineTrack::ASplineTrack()
{
	this->m_Spline = UObject::CreateDefaultSubobject<USplineComponent>(TEXT("SplineTrack"));
	this->m_Spline->SetClosedLoop(true);
	Super::RootComponent = this->m_Spline;
}