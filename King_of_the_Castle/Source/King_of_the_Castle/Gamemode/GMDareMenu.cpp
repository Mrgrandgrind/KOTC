// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMDareMenu.h"


void AGMDareMenu::BeginPlay()
{
	Super::BeginPlay();

	Super::GetWorld()->GetGameViewport()->SetDisableSplitscreenOverride(true);
}
