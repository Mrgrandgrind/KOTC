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
	void KOTC_Respawn();

	UFUNCTION(Exec)
	void KOTC_Stun(const float& duration);

	UFUNCTION(Exec)
	void KOTC_SetTeam(const int& team);

	UFUNCTION(Exec)
	void KOTC_SetHealth(const float& health);

	UFUNCTION(Exec)
	void KOTC_SetStamina(const float& stamina);

	UFUNCTION(Exec)
	void KOTC_SetBlockReach(const float& reach);

	UFUNCTION(Exec)
	void KOTC_SetBlockCount(const int& count);

	UFUNCTION(Exec)
	void KOTC_DebugDrawTrace(const bool& enable);

	UFUNCTION(Exec)
	void KOTC_DebugDrawStructures(const bool& enable);

private:
	class APlayerCharacter* GetCharacter() const;
};
