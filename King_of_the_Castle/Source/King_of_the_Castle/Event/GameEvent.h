// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GameEvent.generated.h"

#define DEFAULT_EVENT_DURATION 30.0f //seconds
#define DEFAULT_EVENT_TRIGGER_CHANCE 0.01f //percentage (0.0f - 1.0f where 0.01f is 1%)
#define DEFAULT_EVENT_TRIGGER_DELAY 1.0f // seconds
#define DEFAULT_EVENT_ACTIVATE_DELAY 5.0f //seconds

UCLASS()
class KING_OF_THE_CASTLE_API AGameEvent : public AActor
{
	GENERATED_BODY()
	
public:
	AGameEvent();

	UFUNCTION(BlueprintPure, Category = "Name")
	virtual FName GetEventName() const { return NAME_None; }

	UFUNCTION(BlueprintPure, Category = "Time")
	const float& GetDuration() const { return this->m_Duration; }

	UFUNCTION(BlueprintPure, Category = "Event")
	const bool& IsRunning() const { return this->m_bRunning; }

	FORCEINLINE virtual void Start() { this->m_bRunning = true; }

	FORCEINLINE virtual void Stop() { this->m_bRunning = false; }

	FORCEINLINE void SetTimer(float *timer) { this->m_Timer = timer; }

protected:
	bool IsFadeActive() const;

	// Returns the current fade percentage where 0.0 = faded out and 1.0 = faded in
	float GetFadePercentage() const;

	// Duration of the complete event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (DisplayName = "Duration"))
	float m_Duration;

	// Whether or not this event has started
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = ( DisplayName = "Running"))
	bool m_bRunning;

	// Variables for derived classes to control the fade in and fade out
	float m_FadeInDuration, m_FadeOutDuration;

private:
	float *m_Timer;
};
