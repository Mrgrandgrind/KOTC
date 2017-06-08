// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameEvent.h"

AGameEvent::AGameEvent() : m_Duration(DEFAULT_EVENT_DURATION),
m_bRunning(false), m_FadeInDuration(0), m_FadeOutDuration(0), m_Timer(nullptr)
{
	Super::PrimaryActorTick.bCanEverTick = false;
}

bool AGameEvent::IsFadeActive() const
{
	if (this->m_Timer == nullptr)
	{
		return false;
	}
	const float& time = *this->m_Timer;
	return time <= this->m_FadeInDuration || time >= this->m_Duration - this->m_FadeOutDuration;
}

float AGameEvent::GetFadePercentage() const
{
	if (this->m_Timer == nullptr)
	{
		return 1.0f;
	}
	const float& time = *this->m_Timer;
	if (time <= this->m_FadeInDuration)
	{
		//checkf(this->m_FadeInDuration <= 0.0f, TEXT("[GameEvent] Somehow we got here with fade in duration <= 0?"));
		return FMath::Sin((3.14159f / 2.0f) * time / this->m_FadeInDuration);
	}
	if (time >= this->m_Duration - this->m_FadeOutDuration)
	{
		if (this->m_FadeOutDuration <= 0.0f) //divide by zero or negative error check
		{
			return 0.0f;
		}
		return FMath::Cos((3.14159f / 2.0f) * (time - (this->m_Duration - this->m_FadeOutDuration)) / this->m_FadeOutDuration);
	}
	return 1.0f;
}
