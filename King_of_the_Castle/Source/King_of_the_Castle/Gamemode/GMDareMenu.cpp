// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMDareMenu.h"

void AGMDareMenu::BeginPlay()
{
	Super::BeginPlay();

	for(int i = 0; i < 4; i++)
	{
		APlayerController *controller = UGameplayStatics::GetPlayerController(Super::GetWorld(), i);
		if (controller == nullptr)
		{
			controller = UGameplayStatics::CreatePlayer(Super::GetWorld(), -1, true);
		}
		check(controller != nullptr);
		controller->SetIgnoreMoveInput(true);
	}

	Super::GetWorld()->GetGameViewport()->SetDisableSplitscreenOverride(true);
}
