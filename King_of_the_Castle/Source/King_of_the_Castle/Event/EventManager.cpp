#include "King_of_the_Castle.h"
#include "EventManager.h"

#include "GameEvent.h"

AEventManager::AEventManager() : m_Event(nullptr)
{
	this->m_EventTimer = 0.0f;
	this->m_EventNameId = NAME_None;
	this->m_NextEventId = -1;
	this->m_EventTriggerChance = DEFAULT_EVENT_TRIGGER_CHANCE;

#if WITH_EDITOR
	this->m_bDebugStopEvent = false;
#endif

	Super::PrimaryActorTick.bCanEverTick = true;
}

void AEventManager::Tick(float delta)
{
	Super::Tick(delta);

	this->m_EventTimer += delta;
	if (this->m_Event != nullptr)
	{
		// Game will tick the actor for us

		if (this->m_EventTimer > this->m_Event->GetDuration())
		{
			this->TriggerEvent(-1);
		}
	}
	else if (this->m_NextEventId < 0) // If there is no event has been triggered yet
	{
		if (this->m_EventTimer >= DEFAULT_EVENT_TRIGGER_DELAY)
		{
#if KOTC_EVENTS_ENABLED
			if (FMath::FRand() < this->m_EventTriggerChance)
			{
				// If trigger was successful and we want an event to happen. 
				// We will just pick a completely random event from our enum. (This can technically be 'None', in which case nothing will happen)
				this->m_NextEventId = FMath::RandRange(0, this->m_EventList.Num() - 1);
			}
#endif
			this->m_EventTimer = 0.0f;
		}
	}
	else // If an event has been triggered and we are just waiting before we activate it
	{
		if (this->m_EventTimer >= DEFAULT_EVENT_ACTIVATE_DELAY)
		{
			this->TriggerEvent(this->m_NextEventId);
			this->m_NextEventId = -1;

			this->m_EventTimer = 0.0f;
		}
	}
}

void AEventManager::TriggerEvent(const int& gameEvent)
{
	if (this->m_Event != nullptr)
	{
		UE_LOG(LogClass, Log, TEXT("[DefaultGameMode] Stopping event: %s"), *this->m_Event->GetEventName().ToString());
		this->m_Event->Stop();
		this->m_Event->Destroy();
	}
	if (gameEvent >= 0 && gameEvent < this->m_EventList.Num() && this->m_EventList[gameEvent] != nullptr)
	{
		this->m_Event = Super::GetWorld()->SpawnActor<AGameEvent>(this->m_EventList[gameEvent]);
	}
	else
	{
		this->m_Event = nullptr;
	}
	if (this->m_Event == nullptr)
	{
		this->m_EventNameId = NAME_None;
	}
	else
	{
		UE_LOG(LogClass, Log, TEXT("[DefaultGameMode] Starting event: %s"), *this->m_Event->GetEventName().ToString());

		this->m_EventNameId = this->m_Event->GetEventName();
		this->m_EventTimer = 0.0f;
		this->m_Event->SetTimer(&this->m_EventTimer);
		this->m_Event->Start();
	}
}

#if WITH_EDITOR
void AEventManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty == nullptr)
	{
		return;
	}
	FName name = PropertyChangedEvent.MemberProperty->GetFName();
	if (name == GET_MEMBER_NAME_CHECKED(AEventManager, m_bDebugStopEvent)
		|| name == GET_MEMBER_NAME_CHECKED(AEventManager, m_NextEventId))
	{
		if (this->m_Event != nullptr)
		{
			this->m_Event->Stop();
			this->m_Event->Destroy();
			this->m_Event = nullptr;
			this->m_EventNameId = NAME_None;
		}
		this->m_EventTimer = 0.0f;
		this->m_bDebugStopEvent = false;
	}
}
#endif

