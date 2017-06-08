// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "BaseGameMode.generated.h"

#define GAME_EVENT_COUNT 3 //should match how many events there are below. must be > 0

UENUM()
enum class EGameEvent : uint8
{
	None, //None should stay at index 0
	FloorIsLava,
	LowGravity
};

UCLASS()
class KING_OF_THE_CASTLE_API ABaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABaseGameMode();

	virtual void Tick(float delta) override;

	// Force an event to happen. There is no count down if you manually trigger it.
	void TriggerEvent(const EGameEvent& gameEvent);

	// True if there is currently an event active. (does not account for triggered events)
	UFUNCTION(BlueprintPure, Category = "Event")
	bool IsEventActive() const { return this->m_Event != nullptr; }

	// Get the user interface text that should be displayed
	UFUNCTION(BlueprintPure, Category = "Event")
	FString GetEventText() const;

	// True if an event is about to happen. The event activates after a period of time.
	UFUNCTION(BlueprintPure, Category = "Event")
	bool IsEventTriggered() const { return this->m_NextEventId != EGameEvent::None; }

	// Time since the current event started, or since it was triggered
	UFUNCTION(BlueprintPure, Category = "Event")
	const float& GetEventTime() const { return this->m_EventTimer; }

	// Get the active event (nullptr if there is no event running)
	UFUNCTION(BlueprintPure, Category = "Event")
	class AGameEvent* GetEvent() const { return this->m_Event; }

	// Get the enum for the active event (EGameEvent::None if no event is active)
	UFUNCTION(BlueprintPure, Category = "Event")
	const EGameEvent& GetEventId() const { return this->m_EventId; }

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
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// Time since game started
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Timer"))
	float m_Timer;

	// How long a game should last
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (AllowPrivateAccess = "true", DisplayName = "Game Duration"))
	float m_GameDuration;

	// Timer used for several things; duration event has been active, duration since last trigger roll, duration since successful roll
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "Event Timer"))
	float m_EventTimer;

	// The percentage chance (0.0f - 1.0f) that an event will occur. This chance is rolled every second.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "Event Trigger Chance"))
	float m_EventTriggerChance;

	// Actor of the current game event that's running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "Current Event"))
	AGameEvent *m_Event;

	// Id of the current game event that's running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "Current Event Id"))
	EGameEvent m_EventId;

	// The next game event which will begin within a specified time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "Next Event"))
	EGameEvent m_NextEventId;
	
// DEBUG VARIABLES //
	// Force an event to be triggered. There is no count down.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Trigger Event"))
	EGameEvent m_DebugTriggerEvent;

	// Force the active event to stop. Does nothing if there's no active event.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "[Debug] Stop Event"))
	bool m_bDebugStopEvent;
/////////////////////
};
