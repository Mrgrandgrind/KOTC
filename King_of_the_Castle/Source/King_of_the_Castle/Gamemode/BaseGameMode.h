// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "BaseGameMode.generated.h"


UCLASS()
class KING_OF_THE_CASTLE_API ABaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABaseGameMode();

	virtual void Tick(float delta) override;

	// Time since the game started
	UFUNCTION(BlueprintPure, Category = "Time")
	const float& GetTime() const { return this->m_Timer; }

	// How long a game should last
	UFUNCTION(BlueprintPure, Category = "Time")
	const float& GetGameDuration() const { return this->m_GameDuration; }

	// Set how long a game should last
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetGameDuration(const float& duration) { this->m_GameDuration = duration; }

	// This function is called whenever a block is placed. Implementation is for blueprints.
	UFUNCTION(BlueprintImplementableEvent)
	void OnBlockPlace(class ABlock *block, const int& team, const int& pointValue);

	// This function is called whenever a block is destroyed. Implementation is for blueprints.
	UFUNCTION(BlueprintImplementableEvent)
	void OnBlockDestroy(class AActor *source, const int& team, const int& pointValue);
	
protected:
	// Time since game started
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Timer"))
	float m_Timer;

	// How long a game should last
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Game Duration"))
	float m_GameDuration;
	
};
