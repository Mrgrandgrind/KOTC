#include "King_of_the_Castle.h"
#include "BaseGameMode.h"

#include "HUD/GameHUD.h"
#include "Character/SpawnPoint.h"
#include "Character/PlayerCharacter.h"
#include "Event/EventManager.h"
#include "Construction/Block.h"
#include "Construction/BlockEntity.h"
#include "Construction/BlockStructureManager.h"
#include "Character/DefaultPlayerController.h"

#include "Runtime/Engine/Public/EngineUtils.h"

#define DEFAULT_GAME_DURATION 10.0f * 60.0f //seconds

#define MAX_BLOCK_ENTITY_COUNT 125
#define BLOCK_ENTITY_DESTROY_BLOCK_COUNT 5 // How many block entities to destroy once we hit the limit

#define MAX_PLAYER_COUNT 4

#define SMASH_CHARACTER_LOCATION TEXT("/Game/Blueprints/Characters/BP_SmashCharacter")

ABaseGameMode::ABaseGameMode() : m_Timer(0.0f), m_GameDuration(DEFAULT_GAME_DURATION), m_MaxEntityCount(MAX_BLOCK_ENTITY_COUNT),
m_EntityCount(0), m_BlockStructureManager(nullptr), m_PlayerCount(1)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(SMASH_CHARACTER_LOCATION);
	if (PlayerPawnBPClass.Class != nullptr)
	{
		Super::DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	Super::HUDClass = AGameHUD::StaticClass();
	Super::PlayerControllerClass = ADefaultPlayerController::StaticClass();

	Super::PrimaryActorTick.bCanEverTick = true;

	this->m_GameDuration = DEFAULT_GAME_DURATION;
	this->m_CharacterClass = PlayerPawnBPClass.Class;
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

	// Spawn players
	this->SpawnPlayers();
}

void ABaseGameMode::Tick(float delta)
{
	Super::Tick(delta);

	this->m_Timer += delta;

	if (this->m_Timer >= this->m_GameDuration)
	{

	}

	this->m_FPS = 1.0f / delta;
}

bool ABaseGameMode::GetSpawnPoint(const int& team, FVector& outLocation, FRotator& outRotation) const
{
	TArray<ASpawnPoint*> team1, team2;
	this->GetSpawnPoints(team1, team2);
	
	ASpawnPoint *point = nullptr;
	if (team <= 1 && team1.Num() > 0)
	{
		point = team1[FMath::RandRange(0, team1.Num() - 1)];
	}
	else if (team >= 2 && team2.Num() > 0)
	{
		point = team2[FMath::RandRange(0, team2.Num() - 1)];
	}
	if (point != nullptr)
	{
		outLocation = point->GetActorLocation();
		outRotation = point->GetActorRotation();
		return true;
	}
	return false;
}

void ABaseGameMode::GetSpawnPoints(TArray<class ASpawnPoint*>& team1, TArray<class ASpawnPoint*>& team2) const
{
	// Get spawn points
	TArray<AActor*> out;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ASpawnPoint::StaticClass(), out);

	// Separate the spawn points into teams
	for (int i = 0; i < out.Num(); i++)
	{
		ASpawnPoint *next = Cast<ASpawnPoint>(out[i]);
		if (next->GetTeam() <= 1 || next->IsTeamIgnored())
		{
			team1.Add(next);
		}
		if (next->GetTeam() >= 2 || next->IsTeamIgnored())
		{
			team2.Add(next);
		}
	}
}

void ABaseGameMode::SpawnPlayers()
{
	// Get spawn points
	TArray<ASpawnPoint*> team1, team2;
	this->GetSpawnPoints(team1, team2);

	// TODO Team selection. Teams are temporary hard coded in.
	auto GetTeam = [this](const int& num)->int
	{
		if (this->m_PlayerCount == 1)
		{
			return 1;
		}
		else if (this->m_PlayerCount == 2 || this->m_PlayerCount == 3)
		{
			return num == 0 ? 1 : 2;
		} 
		else if (this->m_PlayerCount == 4)
		{
			return num <= 1 ? 1 : 2;
		}
		return -1;
	};

	// Spawn players
	for (int i = 0, t1Idx = 0, t2Idx = 0; i < this->m_PlayerCount; i++)
	{
		APlayerCharacter *character = Super::GetWorld()->SpawnActor<APlayerCharacter>(this->m_CharacterClass);
		character->SetTeam(GetTeam(i));

		ASpawnPoint *point = nullptr;
		if (character->GetTeam() <= 1 && team1.Num() > 0)
		{
			point = team1[t1Idx++ % team1.Num()];
		}
		else if (character->GetTeam() >= 2 && team2.Num() > 0)
		{
			point = team2[t2Idx++ % team2.Num()];
		}
		if (point != nullptr)
		{
			character->SetActorLocation(this->m_bUseDefaultStart ? this->m_DefaultStartLocation
				+ FVector(0.0f, 0.0f, 250.0f) * i : point->GetActorLocation());
			character->SetActorRotation(this->m_bUseDefaultStart ? this->m_DefaultStartRotation
				: point->GetActorRotation());
		}

		APlayerController *controller;
		if (i == 0)
		{
			controller = UGameplayStatics::GetPlayerController(Super::GetWorld(), 0);
		}
		else
		{
			controller  = UGameplayStatics::CreatePlayer(Super::GetWorld(), -1, true);
		}
		controller->Possess(character);
	}
}

void ABaseGameMode::OnBlockPlace(ABlock *block, AActor *source)
{
	if(block->IsA(ABlockEntity::StaticClass()))
	{
		this->m_EntityCount++;
		if(this->m_EntityCount - this->m_EntityDespawnFlags > this->m_MaxEntityCount)
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
		this->m_EntityCount--;
		if (this->m_EntityDespawnFlags > 0)
		{
			this->m_EntityDespawnFlags--;
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
		this->m_EntityDespawnFlags++;
		Cast<ABlockEntity>(out[i])->ForceDespawn();
	}
}

APawn* ABaseGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot)
{
	this->m_DefaultStartLocation = StartSpot->GetActorLocation();
	this->m_DefaultStartRotation = StartSpot->GetActorRotation();
	this->m_DefaultStartRotation.Pitch = 0.0f;
	this->m_DefaultStartRotation.Yaw = 0.0f;
	this->m_bUseDefaultStart = !StartSpot->IsA(ASpawnPoint::StaticClass());
	return nullptr;
}

#if WITH_EDITOR
void ABaseGameMode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty == nullptr)
	{
		return;
	}
	FName name = PropertyChangedEvent.MemberProperty->GetFName();
	if (name == GET_MEMBER_NAME_CHECKED(ABaseGameMode, m_PlayerCount))
	{
		this->m_PlayerCount = FMath::Max(FMath::Min(this->m_PlayerCount, MAX_PLAYER_COUNT), 1);
	}
}
#endif