// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "GMMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API AGMMainMenu : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGMMainMenu();

	virtual void BeginPlay() override;

	virtual void Tick(float delta) override;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot) override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (DisplayName = "Duration"))
	float m_Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (DisplayName = "Start Time"))
	float m_StartTime;

private:
	float m_Counter;

	UPROPERTY()
	class ASplineTrack *m_Track;

	UPROPERTY()
	class ADefaultPlayerCharacter *m_Character;
};
