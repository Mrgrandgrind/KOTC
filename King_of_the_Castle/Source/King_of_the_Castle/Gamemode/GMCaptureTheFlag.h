// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Gamemode/BaseGameMode.h"
#include "GMCaptureTheFlag.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API AGMCaptureTheFlag : public ABaseGameMode
{
	GENERATED_BODY()
	
public:
	AGMCaptureTheFlag();

	FORCEINLINE class APlayerCharacter* GetFlagHolder() const { return this->m_FlagHolder; }

	FORCEINLINE void SetFlagHolder(class APlayerCharacter *character) { this->m_FlagHolder = character; }

private:
	class APlayerCharacter *m_FlagHolder;
	
};
