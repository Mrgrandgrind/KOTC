// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "King_of_the_Castle.h"
#include "King_of_the_CastleGameMode.h"
#include "King_of_the_CastleCharacter.h"

AKing_of_the_CastleGameMode::AKing_of_the_CastleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
