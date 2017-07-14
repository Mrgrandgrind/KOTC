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

	virtual void BeginPlay() override;

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

	virtual void OnBlockPlace(class ABlock *block, AActor *source);

	virtual void OnBlockDestroy(class ABlock *block, AActor *source);

	virtual void OnBlockPickup(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount) { }

	virtual void OnBlockDrop(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount) { }

	FORCEINLINE class AEventManager* GetEventManager() const { return this->m_EventManager; }

	FORCEINLINE class ABlockStructureManager* GetStructureManager() const { return this->m_BlockStructureManager; }

protected:
	void RemoveBlockEntities(const int& count);

private:
	// How many players should be ingame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players", meta = (AllowPrivateAccess = "true", DisplayName = "Player Count"))
	int m_PlayerCount;

	// Time since game started (seconds)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Timer"))
	float m_Timer;

	// How long a game should last (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Game Duration"))
	float m_GameDuration;

	// How many block entities can exist in the game at a given time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Max Block Entity Count"))
	int m_MaxBlockEntityCount;

	// Block Entites spawn control
	int m_BlockEntityCount, m_BlockEntityDespawnFlags;

	// Event manager reference
	UPROPERTY()
	class AEventManager *m_EventManager;
	
	// Structure manager reference
	UPROPERTY()
	class ABlockStructureManager *m_BlockStructureManager;
};
