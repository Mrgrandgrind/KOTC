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

	//virtual void OnBlockPlace(class ABlock *block, AActor *source) override;

	void OnBlockPlace_Implementation(class ABlock *block, AActor *source) override;

	void OnBlockDestroy_Implementation(class ABlock *block, AActor *source) override;

	void OnBlockPickup_Implementation(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount) override;

	void OnBlockDrop_Implementation(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount) override;

	virtual FName GetGameModeName() const override { return TEXT("Capture The Flag"); }

	FORCEINLINE class APlayerCharacter* GetFlagHolder(const int& team) const
	{
		if(team == 1)
		{
			return this->m_FlagHolderTeam1;
		}
		if(team == 2)
		{
			return this->m_FlagHolderTeam2;
		}
		return nullptr;
	}

private:
	class APlayerCharacter *m_FlagHolderTeam1, *m_FlagHolderTeam2;
};
