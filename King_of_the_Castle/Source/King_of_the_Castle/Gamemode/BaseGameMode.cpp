#include "King_of_the_Castle.h"
#include "BaseGameMode.h"

#include "HUD/GameHUD.h"
#include "Event/EventManager.h"
#include "Construction/Block.h"
#include "Construction/BlockEntity.h"
#include "Construction/BlockStructureManager.h"
#include "Character/DefaultPlayerController.h"

#include "Runtime/Engine/Public/EngineUtils.h"

#define DEFAULT_GAME_DURATION 10.0f * 60.0f //seconds

#define MAX_BLOCK_ENTITY_COUNT 250
#define BLOCK_ENTITY_DESTROY_BLOCK_COUNT 5 // How many block entities to destroy once we hit the limit

ABaseGameMode::ABaseGameMode() : m_Timer(0.0f), m_GameDuration(DEFAULT_GAME_DURATION), m_MaxBlockEntityCount(MAX_BLOCK_ENTITY_COUNT),
m_BlockEntityCount(0), m_BlockStructureManager(nullptr)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/BP_SmashCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		Super::DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	Super::HUDClass = AGameHUD::StaticClass();
	Super::PlayerControllerClass = ADefaultPlayerController::StaticClass();

	Super::PrimaryActorTick.bCanEverTick = true;

	this->m_GameDuration = DEFAULT_GAME_DURATION;
}

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> out;
	// Find the event manager
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), AEventManager::StaticClass(), out);
	checkf(out.Num() <= 1, TEXT("There should not be more than 1 EventManager actor"));
	if (out.Num() > 0)
	{
		this->m_EventManager = Cast<AEventManager>(out[0]);
	}
	else
	{
		UE_LOG(LogClass, Error, TEXT("No AEventManager placed in map!"));
	}

	// Find the block structure manager
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlockStructureManager::StaticClass(), out);
	checkf(out.Num() <= 1, TEXT("There should not be more than 1 BlockStructureManager actor"));
	if (out.Num() > 0)
	{
		this->m_BlockStructureManager = Cast<ABlockStructureManager>(out[0]);
	}
	else
	{
		UE_LOG(LogClass, Error, TEXT("No ABlockStructureManager placed in map!"));
	}
}

void ABaseGameMode::Tick(float delta)
{
	Super::Tick(delta);

	this->m_Timer += delta;

	if (this->m_Timer >= this->m_GameDuration)
	{

	}
}

void ABaseGameMode::OnBlockPlace(ABlock *block, AActor *source)
{
	if(block->IsA(ABlockEntity::StaticClass()))
	{
		this->m_BlockEntityCount++;
		if(this->m_BlockEntityCount - this->m_BlockEntityDespawnFlags > this->m_MaxBlockEntityCount)
		{
			this->RemoveBlockEntities(BLOCK_ENTITY_DESTROY_BLOCK_COUNT);
		}
	}
	if (this->m_BlockStructureManager != nullptr)
	{
		this->m_BlockStructureManager->ProcessCreate(block);
	}
}

void ABaseGameMode::OnBlockDestroy(ABlock *block, AActor *source)
{
	if (block->IsA(ABlockEntity::StaticClass()))
	{
		this->m_BlockEntityCount--;
		if (this->m_BlockEntityDespawnFlags > 0)
		{
			this->m_BlockEntityDespawnFlags--;
		}
	}
	if (this->m_BlockStructureManager != nullptr)
	{
		this->m_BlockStructureManager->ProcessDestroy(block);
	}
}

void ABaseGameMode::RemoveBlockEntities(const int& count)
{
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlockEntity::StaticClass(), out);

	for (int i = 0; i < FMath::Min(count, out.Num()); i++)
	{
		this->m_BlockEntityDespawnFlags++;
		Cast<ABlockEntity>(out[i])->ForceDespawn();
	}
}