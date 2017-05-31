#include "King_of_the_Castle.h"
#include "BaseGameMode.h"

ABaseGameMode::ABaseGameMode()
{
	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABaseGameMode::Tick(float delta)
{
	Super::Tick(delta);

	this->m_Timer += delta;
}
