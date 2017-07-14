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

	void GetSpawnPoints(TArray<class ASpawnPoint*>& team1, TArray<class ASpawnPoint*>& team2) const;

	void SpawnPlayers();

	virtual void OnBlockPlace(class ABlock *block, AActor *source);

	virtual void OnBlockDestroy(class ABlock *block, AActor *source);

	virtual void OnBlockPickup(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount) { }

	virtual void OnBlockDrop(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount) { }

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot) override;

	// Time since the game started
	FORCEINLINE const float& GetTime() const { return this->m_Timer; }

	// How long a game should last
	FORCEINLINE const float& GetGameDuration() const { return this->m_GameDuration; }

	// Set how long a game should last
	FORCEINLINE void SetGameDuration(const float& duration) { this->m_GameDuration = duration; }

	// Event manager reference
	FORCEINLINE class AEventManager* GetEventManager() const { return this->m_EventManager; }

	// Structure manager reference
	FORCEINLINE class ABlockStructureManager* GetStructureManager() const { return this->m_BlockStructureManager; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void RemoveBlockEntities(const int& count);

	FVector m_DefaultStartLocation;

	FRotator m_DefaultStartRotation;

	bool m_bUseDefaultStart;

	// How many players should be ingame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players", meta = (AllowPrivateAccess = "true", DisplayName = "Player Count"))
	int m_PlayerCount;

	UPROPERTY()
	TSubclassOf<class APlayerCharacter> m_CharacterClass;

	// Time since game started (seconds)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Timer"))
	float m_Timer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "FPS"))
	float m_FPS;

	// How long a game should last (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Game Duration"))
	float m_GameDuration;

	// How many block entities can exist in the game at a given time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Max Entity Count"))
	int m_MaxEntityCount;

	// Block Entites spawn control
	int m_EntityCount, m_EntityDespawnFlags;

	// Event manager reference
	UPROPERTY()
	class AEventManager *m_EventManager;
	
	// Structure manager reference
	UPROPERTY()
	class ABlockStructureManager *m_BlockStructureManager;
};
