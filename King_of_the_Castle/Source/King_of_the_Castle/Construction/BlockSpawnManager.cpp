// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BlockSpawnManager.h"

#include "BlockData.h"
#include "BlockSpawnArea.h"

#define DEFAULT_SPAWN_DELAY 0.12f //seconds

#define DATA_BASIC_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_BasicBlock_Data")
#define DATA_GOLD_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_GoldBlock_Data")

ABlockSpawnManager::ABlockSpawnManager() : m_DelayCounter(0.0f)
{
	Super::RootComponent = UObject::CreateDefaultSubobject<USceneComponent>(TEXT("BlockSpawnManager"));

	static ConstructorHelpers::FClassFinder<UBlockData> DataConstruction(DATA_BASIC_LOCATION);
	if (DataConstruction.Succeeded())
	{
		this->m_BasicType = ((UBlockData*)DataConstruction.Class->GetDefaultObject())->GetClassType();
	}

	static ConstructorHelpers::FClassFinder<UBlockData> DataGold(DATA_GOLD_LOCATION);
	if (DataGold.Succeeded())
	{
		this->m_GoldType = ((UBlockData*)DataGold.Class->GetDefaultObject())->GetClassType();
	}

	this->m_GoldBlocks = 5;
	this->m_BasicBlocks = 10;

	this->m_SpawnDelay = DEFAULT_SPAWN_DELAY;

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ABlockSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// Locate all spawn areas in the world
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ABlockSpawnArea::StaticClass(), out);
	for (AActor *next : out)
	{
		this->m_SpawnAreas.Add((ABlockSpawnArea*)next);
	}
	this->m_DelayCounter = this->m_SpawnDelay;
}

void ABlockSpawnManager::Tick(float delta)
{
	Super::Tick(delta);

	// If the game has no spawn areas, we cannot spawn anything. Don't bother continuing with tick.
	if (this->m_SpawnAreas.Num() == 0)
	{
		return;
	}
	this->m_DelayCounter += delta;
	if(this->m_DelayCounter >= this->m_SpawnDelay)
	{
		if (this->m_BasicBlocks > 0 && this->SpawnBlock(this->m_BasicType))
		{
			this->m_BasicBlocks--;
		}
		else if (this->m_GoldBlocks > 0 && this->SpawnBlock(this->m_GoldType))
		{
			this->m_GoldBlocks--;
		}
		this->m_DelayCounter -= this->m_SpawnDelay;
	}
}

bool ABlockSpawnManager::SpawnBlock(TSubclassOf<ABlock> type)
{
	if (type == nullptr)
	{
		// If we failed to load this type. Just pretend like we've spawned it so we stop getting requests.
		return true;
	}
	ABlockSpawnArea *area = this->m_SpawnAreas[FMath::RandRange(0, this->m_SpawnAreas.Num() - 1)];
	checkf(area != nullptr, TEXT("Area is somehow nullptr"));

	// Try getting a position to spawn the block in
	FVector out;
	if (area->TrySpawn(out))
	{
		// Adjust spawn Z
		out.Z += BRUSH_SPAWN_Z_OFFSET;
		// If the position gotten is valid, return whether or not the game spawned it
		return Super::GetWorld()->SpawnActor<ABlock>(type, out, FRotator(0.0f)) != nullptr;
	}
	return false;
}
