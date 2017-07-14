// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "SpawnPoint.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API ASpawnPoint : public APlayerStart
{
	GENERATED_BODY()
	
public:
	ASpawnPoint();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int m_Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Ignore Team"))
	bool m_bIgnoreTeam;
};
