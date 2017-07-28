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
#include "Runtime/Engine/Classes/Engine/Engine.h"

#define DEFAULT_GAME_DURATION 10.0f * 60.0f //seconds

#define MAX_BLOCK_ENTITY_COUNT 125
#define BLOCK_ENTITY_DESTROY_BLOCK_COUNT 5 // How many block entities to destroy once we hit the limit

#define MAX_PLAYER_COUNT 4

#define HUD_LOCATION TEXT("/Game/Blueprints/HUD/BP_GameHUD_CP")
#define SMASH_CHARACTER_LOCATION TEXT("/Game/Blueprints/Characters/BP_RobotCharacter")

#define TEAM1_COLOR FLinearColor(0.4f, 0.4f, 1.0f)
#define TEAM2_COLOR FLinearColor(1.0f, 0.3f, 0.3f)
#define TEAM3_COLOR FLinearColor(0.0f, 0.4f, 0.0f)
#define TEAM4_COLOR FLinearColor(0.4f, 0.0f, 0.4f)

ABaseGameMode::ABaseGameMode() : m_Timer(0.0f), m_GameDuration(DEFAULT_GAME_DURATION), m_MaxEntityCount(MAX_BLOCK_ENTITY_COUNT),
m_EntityCount(0), m_BlockStructureManager(nullptr), m_PlayerCount(1)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(SMASH_CHARACTER_LOCATION);
	if (PlayerPawnBPClass.Class != nullptr)
	{
		Super::DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	static ConstructorHelpers::FClassFinder<AHUD> HUD(HUD_LOCATION);
	if (HUD.Class != nullptr)
	{
		Super::HUDClass = HUD.Class;
	}
	Super::PlayerControllerClass = ADefaultPlayerController::StaticClass();
	
	Super::PrimaryActorTick.bCanEverTick = true;

	this->m_GameDuration = DEFAULT_GAME_DURATION;
	this->m_CharacterClass = PlayerPawnBPClass.Class;

	this->m_TeamColors.Add(TEAM1_COLOR);
	this->m_TeamColors.Add(TEAM2_COLOR);
	this->m_TeamColors.Add(TEAM3_COLOR);
	this->m_TeamColors.Add(TEAM4_COLOR);
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

	this->m_FPS = 1.0f / delta;

	if (this->IsGameOver())
	{
		return;
	}

	this->m_Timer += delta;

	if (this->m_Timer >= this->m_GameDuration && !this->m_bInfiniteTime)
	{
		this->EndGame(FString::Printf(TEXT("Time Limit Reached")));
	}
}

void ABaseGameMode::EndGame(FString message)
{
	this->m_bGameOver = true;
	for (FConstPlayerControllerIterator itr = Super::GetWorld()->GetPlayerControllerIterator(); itr; ++itr)
	{
		AGameHUD *hud = Cast<AGameHUD>((*itr)->GetHUD());
		if (hud != nullptr)
		{
			//hud->SetGameOver(message);
		}
		(*itr)->SetInputMode(FInputModeUIOnly());
	}
}

bool ABaseGameMode::GetSpawnPoint(const int& team, FVector& outLocation, FRotator& outRotation) const
{
	TArray<ASpawnPoint*> points;
	this->GetSpawnPoints(team, points);
	
	ASpawnPoint *point = nullptr;
	if (points.Num() > 0)
	{
		point = points[FMath::RandRange(0, points.Num() - 1)];
	}
	if (point != nullptr)
	{
		outLocation = point->GetActorLocation();
		outRotation = point->GetActorRotation();
		return true;
	}
	return false;
}

void ABaseGameMode::GetSpawnPoints(const int& team, TArray<class ASpawnPoint*>& out) const
{
	out.Empty();

	// Get spawn points
	TArray<AActor*> all;
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ASpawnPoint::StaticClass(), all);

	// Separate the spawn points into teams
	for (int i = 0; i < all.Num(); i++)
	{
		ASpawnPoint *next = Cast<ASpawnPoint>(all[i]);
		if (next->GetTeam() == team || next->IsTeamIgnored())
		{
			out.Add(next);
		}
	}
}

void ABaseGameMode::SpawnPlayers()
{
	// TODO Team selection. Teams are temporary hard coded in.
	auto GetTeam = [this](const int& num)->int
	{
		if (this->m_bFreeForAll)
		{
			return num + 1;
		}
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

	TMap<int, TPair<int, TArray<ASpawnPoint*>>> map;
	// Spawn players
	for (int i = 0; i < this->m_PlayerCount; i++)
	{
		int team = GetTeam(i);

		TPair<int, TArray<ASpawnPoint*>> *points = map.Find(team);
		if (points == nullptr)
		{
			TPair<int, TArray<ASpawnPoint*>> pair;
			this->GetSpawnPoints(team, pair.Value);
			map.Add(team, pair);
			points = map.Find(team);
		}
		check(points != nullptr);

		ASpawnPoint *point = nullptr;
		if (points->Value.Num() > 0)
		{
			point = points->Value[points->Key++ % points->Value.Num()];
		}

		FVector loc = FVector(0.0f);
		FRotator rot = FRotator(0.0f);
		if (point != nullptr)
		{
			loc = point->GetActorLocation();
			rot = point->GetActorRotation();
		}
		if (this->m_bUseDefaultStart)
		{
			loc = this->m_DefaultStartLocation + FVector(0.0f, 0.0f, 250.0f) * i;
			rot = this->m_DefaultStartRotation;
		}

		APlayerCharacter *character = Super::GetWorld()->SpawnActor<APlayerCharacter>(this->m_CharacterClass, loc, rot);
		if (character == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(
				TEXT("Unable to spawn player %d due to a collision!"), i + 1));
			continue;
		}
		character->SetTeam(team);

		if (team > this->m_TeamCount)
		{
			this->m_TeamCount = team;
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
		if (controller == nullptr)
		{
			continue;
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