// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Event/GameEvent.h"
#include "MarbleShowerEvent.generated.h"

#define MARBLE_SHOWER_NAME TEXT("Marble Shower")

UCLASS()
class KING_OF_THE_CASTLE_API AMarbleShowerEvent : public AGameEvent
{
	GENERATED_BODY()
	
public:
	AMarbleShowerEvent();

	virtual void Tick(float delta) override;

	virtual void Start() override;

	virtual void Stop() override;

	virtual FName GetEventName() const override { return MARBLE_SHOWER_NAME; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Radius"))
	float m_Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Origin Position"))
	FVector m_OriginPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Ray Height"))
	float m_RayHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Spawn Delay"))
	float m_SpawnDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (DisplayName = "Marble Class"))
	TSubclassOf<AActor> m_MarbleClass;

private:
	float m_Counter;
};
