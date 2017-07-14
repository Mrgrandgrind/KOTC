#include "King_of_the_Castle.h"
#include "EventManager.h"

#include "GameEvent.h"
#include "LowGravityEvent.h"
#include "FloorIsLavaEvent.h"

AEventManager::AEventManager()
{
	this->m_EventTimer = 0.0f;
	this->m_EventId = EGameEvent::None;
	this->m_NextEventId = EGameEvent::None;
	this->m_EventTriggerChance = DEFAULT_EVENT_TRIGGER_CHANCE;

#if WITH_EDITOR
	this->m_bDebugStopEvent = false;
	this->m_DebugTriggerEvent = EGameEvent::None;
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
			this->TriggerEvent(EGameEvent::None);
		}
	}
	else if (this->m_NextEventId == EGameEvent::None) // If there is no event has been triggered yet
	{
		if (this->m_EventTimer >= DEFAULT_EVENT_TRIGGER_DELAY)
		{
#if KOTC_EVENTS_ENABLED
			if (FMath::FRand() < this->m_EventTriggerChance)
			{
				// If trigger was successful and we want an event to happen. 
				// We will just pick a completely random event from our enum. (This can technically be 'None', in which case nothing will happen)
				this->m_NextEventId = static_cast<EGameEvent>(FMath::RandRange(0, GAME_EVENT_COUNT - 1));
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
			this->m_NextEventId = EGameEvent::None;

			this->m_EventTimer = 0.0f;
		}
	}
}

FString AEventManager::GetEventText() const
{
	if (this->IsEventTriggered())
	{
		auto name = TEXT("Event");
		switch (this->m_NextEventId)
		{
			case EGameEvent::FloorIsLava:
				name = TEXT("Floor will be lava in");
				break;
			case EGameEvent::LowGravity:
				name = TEXT("Low gravity in");
				break;
			default: break;
		}
		return FString::Printf(TEXT("%s %d"), name, FMath::RoundToInt(5.0f - this->GetEventTime()));
	}
	return TEXT("");
}

void AEventManager::TriggerEvent(const EGameEvent& gameEvent)
{
	if (this->m_Event != nullptr)
	{
		UE_LOG(LogClass, Log, TEXT("[DefaultGameMode] Stopping event: %s"), *this->m_Event->GetEventName().ToString());
		this->m_Event->Stop();
		this->m_Event->Destroy();
	}
	switch (gameEvent)
	{
		case EGameEvent::None:
			this->m_Event = nullptr;
			break;
		case EGameEvent::FloorIsLava:
			this->m_Event = Super::GetWorld()->SpawnActor<AFloorIsLavaEvent>();
			break;
		case EGameEvent::LowGravity:
			this->m_Event = Super::GetWorld()->SpawnActor<ALowGravityEvent>();
			break;
		default:
			this->m_Event = nullptr;
			UE_LOG(LogClass, Error, TEXT("[DefaultGameMode] Undefined GameEvent: %d"), (int)gameEvent);
			return;
	}
	this->m_EventId = gameEvent;
	if (this->m_Event != nullptr)
	{
		UE_LOG(LogClass, Log, TEXT("[DefaultGameMode] Starting event: %s"), *this->m_Event->GetEventName().ToString());
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
	if (name == GET_MEMBER_NAME_CHECKED(AEventManager, m_DebugTriggerEvent))
	{
		this->TriggerEvent(this->m_DebugTriggerEvent);
		this->m_NextEventId = EGameEvent::None;
		this->m_DebugTriggerEvent = EGameEvent::None;
	}
	if (name == GET_MEMBER_NAME_CHECKED(AEventManager, m_bDebugStopEvent)
		|| name == GET_MEMBER_NAME_CHECKED(AEventManager, m_NextEventId))
	{
		if (this->m_Event != nullptr)
		{
			this->m_Event->Stop();
			this->m_Event->Destroy();
			this->m_Event = nullptr;
			this->m_EventId = EGameEvent::None;
		}
		this->m_EventTimer = 0.0f;
		this->m_bDebugStopEvent = false;
	}
}
#endif

