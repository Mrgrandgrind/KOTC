// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "EventManager.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API AEventManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEventManager();

	virtual void Tick(float delta) override;

	// Force an event to happen. There is no count down if you manually trigger it.
	void TriggerEvent(const int& idx);

	FORCEINLINE bool IsEventActive() const { return this->m_Event != nullptr; }

	FORCEINLINE bool IsEventTriggered() const { return this->m_NextEventId >= 0; }

	FORCEINLINE const float& GetEventTime() const { return this->m_EventTimer; }

	FORCEINLINE class AGameEvent* GetEvent() const { return this->m_Event; }

	FORCEINLINE const FName& GetEventNameId() const { return this->m_EventNameId; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Event List"))
	TArray<TSubclassOf<class AGameEvent>> m_EventList;

	// Timer used for several things; duration event has been active, duration since last trigger roll, duration since successful roll
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (DisplayName = "Event Timer"))
	float m_EventTimer;

	// The percentage chance (0.0f - 1.0f) that an event will occur. This chance is rolled every second.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Event Trigger Chance"))
	float m_EventTriggerChance;

	// Actor of the current game event that's running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (DisplayName = "Current Event"))
	AGameEvent *m_Event;

	// Id of the current game event that's running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (DisplayName = "Current Event Id"))
	FName m_EventNameId;

	// The next game event which will begin within a specified time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Next Event Idx"))
	int m_NextEventId;

public:
// DEBUG VARIABLES //
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "[Debug] Start Event"))
	int m_DebugStartEvent;

	// Force the active event to stop. Does nothing if there's no active event.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "[Debug] Stop Event"))
	bool m_bDebugStopEvent;
/////////////////////
};
