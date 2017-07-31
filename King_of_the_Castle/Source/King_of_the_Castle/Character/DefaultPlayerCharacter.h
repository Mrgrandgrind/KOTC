// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "DefaultPlayerCharacter.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ADefaultPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADefaultPlayerCharacter();

	FORCEINLINE class UCameraComponent* GetCamera() { return this->m_Camera; }

private:
	class UCameraComponent *m_Camera;
};
