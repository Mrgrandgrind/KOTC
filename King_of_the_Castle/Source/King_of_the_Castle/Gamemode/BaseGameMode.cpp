#include "King_of_the_Castle.h"
#include "BaseGameMode.h"

#define DEFAULT_GAME_DURATION 600.0f

ABaseGameMode::ABaseGameMode(): m_Timer(0.0f), m_GameDuration(DEFAULT_GAME_DURATION)
{
	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABaseGameMode::Tick(float delta)
{
	Super::Tick(delta);

	this->m_Timer += delta;

	if(this->m_Timer >= this->m_GameDuration)
	{
		
	}
}
