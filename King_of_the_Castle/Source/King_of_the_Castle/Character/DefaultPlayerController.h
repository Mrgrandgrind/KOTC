// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Exec)
	void KOTC_Stun();

	UFUNCTION(Exec)
	void KOTC_SetTeam(const int& team);

	UFUNCTION(Exec)
	void KOTC_SetBlockReach(const float& reach);

	UFUNCTION(Exec)
	void KOTC_SetBlockCount(const int& count);

private:
	class APlayerCharacter* GetCharacter() const;
};
