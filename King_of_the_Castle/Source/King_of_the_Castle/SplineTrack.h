// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SplineTrack.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ASplineTrack : public AActor
{
	GENERATED_BODY()
	
public:	
	ASplineTrack();

	FORCEINLINE class USplineComponent* GetSpline() const { return this->m_Spline; }

	FORCEINLINE FVector GetPivot() const { return this->m_Pivot == nullptr ? FVector() : this->m_Pivot->GetActorLocation(); }

protected:
	UPROPERTY(EditAnywhere, Category = "Spline", meta = (DisplayName = "Spline Component"))
	class USplineComponent *m_Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (DisplayName = "Look At Pivot"))
	AActor *m_Pivot;
};
