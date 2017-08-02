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

	bool GetSpawnPoint(const int& team, FVector& outLocation, FRotator& outRotation) const;

	void GetSpawnPoints(const int& team, TArray<class ASpawnPoint*>& out) const;

	void SpawnPlayers();

	void EndGame(FString message);

	UFUNCTION(BlueprintNativeEvent)
	void OnBlockPlace(class ABlock *block, AActor *source);

	UFUNCTION(BlueprintNativeEvent)
	void OnBlockDestroy(class ABlock *block, AActor *source);

	UFUNCTION(BlueprintNativeEvent)
	void OnBlockPickup(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount);

	UFUNCTION(BlueprintNativeEvent)
	void OnBlockDrop(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount);

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot) override;

	virtual FName GetGameModeName() const { return TEXT("Undefined"); }

	UFUNCTION(BlueprintPure, Category = "GameOver")
	const bool& IsGameOver() const { return this->m_bGameOver; }

	// Time since the game started
	FORCEINLINE const float& GetTime() const { return this->m_Timer; }

	// How long a game should last
	FORCEINLINE const float& GetGameDuration() const { return this->m_GameDuration; }

	// How many players are ingame
	FORCEINLINE const int& GetPlayerCount() const { return this->m_PlayerCount; }

	// How many different teams there are
	FORCEINLINE const int& GetTeamCount() const { return this->m_TeamCount; }

	// Does this game have infinite time
	FORCEINLINE const bool& IsTimerDisabled() const { return this->m_bDisableTimer; }

	// Set how long a game should last
	FORCEINLINE void SetGameDuration(const float& duration) { this->m_GameDuration = duration; }

	// Event manager reference
	FORCEINLINE class AEventManager* GetEventManager() const { return this->m_EventManager; }

	// Structure manager reference
	FORCEINLINE class ABlockStructureManager* GetStructureManager() const { return this->m_BlockStructureManager; }

	// Get team color
	FORCEINLINE const FLinearColor& GetTeamColor(const int& team) const 
	{ 
		return team > 0 && team <= this->m_TeamColors.Num() ? this->m_TeamColors[team - 1] : FLinearColor::White; 
	}

	FORCEINLINE int32 GetPlace(const int& team)
	{
		if (!this->m_TeamScores.Contains(team))
		{
			return this->m_TeamCount;
		}
		const float& score = this->m_TeamScores[team];
		int placement = 1;
		for (auto next : this->m_TeamScores)
		{
			if (next.Key == team)
			{
				continue;
			}
			if (next.Value > score)
			{
				placement++;
			}
		}
		return placement;
	}

	// Get the score of team
	UFUNCTION(BlueprintCallable, Category = "Score")
	int GetScore(const int& team) 
	{
		if (!this->m_TeamScores.Contains(team))
		{
			return 0;
		}
		return int(this->m_TeamScores[team]);
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	FORCEINLINE TMap<int, float>& GetScores() { return this->m_TeamScores; }

private:
	void RemoveBlockEntities(const int& count);

	FVector m_DefaultStartLocation;

	FRotator m_DefaultStartRotation;

	bool m_bUseDefaultStart;

	TMap<int, float> m_TeamScores;
	
	int m_TeamCount;

	bool m_bGameOver;

	// How many players should be ingame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players", meta = (AllowPrivateAccess = "true", DisplayName = "Player Count"))
	int m_PlayerCount;

	// Spawn everyone on a new team
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players", meta = (AllowPrivateAccess = "true", DisplayName = "Free For All"))
	bool m_bFreeForAll;

	// Team colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players", meta = (AllowPrivateAccess = "true", DisplayName = "Team Colors"))
	TArray<FLinearColor> m_TeamColors;

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

	// How long a game should last (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Disable Game Timer"))
	bool m_bDisableTimer;

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
