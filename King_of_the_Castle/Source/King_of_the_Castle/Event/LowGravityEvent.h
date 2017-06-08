// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Event/GameEvent.h"
#include "LowGravityEvent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API ALowGravityEvent : public AGameEvent
{
	GENERATED_BODY()
	
public:
	ALowGravityEvent();

	virtual void Start() override;

	virtual void Stop() override;

	UFUNCTION(BlueprintPure, Category = "Name")
	virtual FName GetEventName() const override { return TEXT("Low Gravity"); }

private:
	// The gravity value before we modified it. We will put it back to this afterwards.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity", meta = (AllowPrivateAccess = "true", DisplayName = "Gravity Cache"))
	float m_GravityCache;	
};
