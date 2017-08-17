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
#include "HUD/Components/CountdownComponent.h"

#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#define DEFAULT_GAME_DURATION 5.0f * 60.0f //seconds

#define MAX_BLOCK_ENTITY_COUNT 125
#define BLOCK_ENTITY_DESTROY_BLOCK_COUNT 5 // How many block entities to destroy once we hit the limit

#define MAX_PLAYER_COUNT 4

#define HUD_LOCATION TEXT("/Game/Blueprints/HUD/BP_GameHUD_CP")
#define CHARACTER_LOCATION TEXT("/Game/Blueprints/Characters/BP_RobotCharacter")

#define TEAM1_MAIN_COLOR FLinearColor(0.17f, 0.57f, 0.71f)
#define TEAM1_EYE_COLOR FLinearColor(0.17f, 0.03f, 0.24f)
#define TEAM1_JOINT_COLOR FLinearColor(0.9f, 0.86f, 0.36f)

#define TEAM2_MAIN_COLOR FLinearColor(0.75f, 0.18f, 0.4f)
#define TEAM2_EYE_COLOR FLinearColor(0.24f, 0.23f, 0.03f)
#define TEAM2_JOINT_COLOR FLinearColor(0.35f, 0.75f, 0.9f)

#define TEAM3_MAIN_COLOR FLinearColor(0.15f, 0.71f, 0.33f)
#define TEAM3_EYE_COLOR FLinearColor(0.03f, 0.09f, 0.23f)
#define TEAM3_JOINT_COLOR FLinearColor(0.91f, 0.39f, 0.39f)

#define TEAM4_MAIN_COLOR FLinearColor(0.79f, 0.31f, 0.19f)
#define TEAM4_EYE_COLOR FLinearColor(0.11f, 0.22f, 0.02f)
#define TEAM4_JOINT_COLOR FLinearColor(0.31f, 0.38f, 0.87f)

ABaseGameMode::ABaseGameMode() : m_PlayerCount(1), m_Timer(0.0f), m_GameDuration(DEFAULT_GAME_DURATION),
m_MaxEntityCount(MAX_BLOCK_ENTITY_COUNT), m_EntityCount(0), m_BlockStructureManager(nullptr)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(CHARACTER_LOCATION);
	Super::DefaultPawnClass = PlayerPawnBPClass.Class;

	static ConstructorHelpers::FClassFinder<AHUD> HUD(HUD_LOCATION);
	Super::HUDClass = HUD.Class;

	Super::PlayerControllerClass = ADefaultPlayerController::StaticClass();

	this->m_bCountdown = true;
	this->m_GameDuration = DEFAULT_GAME_DURATION;
	this->m_CharacterClass = PlayerPawnBPClass.Class;

	this->m_TeamColors.Add({ TEAM1_MAIN_COLOR, TEAM1_EYE_COLOR, TEAM1_JOINT_COLOR });
	this->m_TeamColors.Add({ TEAM2_MAIN_COLOR, TEAM2_EYE_COLOR, TEAM2_JOINT_COLOR });
	this->m_TeamColors.Add({ TEAM3_MAIN_COLOR, TEAM3_EYE_COLOR, TEAM3_JOINT_COLOR });
	this->m_TeamColors.Add({ TEAM4_MAIN_COLOR, TEAM4_EYE_COLOR, TEAM4_JOINT_COLOR });

	Super::PrimaryActorTick.bCanEverTick = true;
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
		this->m_EventManager->SetActive(true);
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

void ABaseGameMode::SetTeamColors(const int& team, UMaterialInstanceDynamic *material)
{
	// If invalid range
	if (team - 1 < 0 || team - 1 >= this->m_TeamColors.Num()) 
	{
		return;
	}
	const FTeamColor& color = this->m_TeamColors[team - 1];
	material->SetVectorParameterValue(TEXT("Team Color"), color.main);
	material->SetVectorParameterValue(TEXT("Eye Color"), color.eye);
	material->SetVectorParameterValue(TEXT("Joint Color"), color.joint);
}

void ABaseGameMode::Tick(float delta)
{
	Super::Tick(delta);

	this->m_FPS = 1.0f / delta;

	if (this->IsGameOver())
	{
		FConstPlayerControllerIterator itr = Super::GetWorld()->GetPlayerControllerIterator();
		// Check to see if we're ready to restart
		for (; itr; ++itr)
		{
			//if ((*itr)->IsInputKeyDown(EKeys::G))
			//	break;
			AGameHUD *hud = Cast<AGameHUD>(itr->Get()->GetHUD());
			if (hud != nullptr && !hud->IsGameOverReady())
			{
				// Someone isn't ready to restart
				return;
			}
		}
		itr.Reset();
		for (; itr; ++itr)
		{
			Super::GetWorldTimerManager().ClearAllTimersForObject(itr->Get()->GetPawn());
		}
		UGameplayStatics::OpenLevel(Super::GetWorld(), TEXT("Main_Level"));
		return;
	}

	if (this->m_bCountdown)
	{
		FConstPlayerControllerIterator itr = Super::GetWorld()->GetPlayerControllerIterator();

		bool active = false;
		for (; itr; ++itr)
		{
			UCountdownComponent *component = AGameHUD::FindComponent<UCountdownComponent>(itr->Get());
			if (component != nullptr && !component->IsCountdownComplete())
			{
				active = true;
				break;
			}
		}
		if (!active)
		{
			this->m_bCountdown = false;

			itr.Reset();
			for (; itr; ++itr)
			{
				itr->Get()->SetIgnoreMoveInput(false);
			}
		}
		return;
	}
	if (!this->m_bDisableTimer)
	{
		this->m_Timer += delta;
	}
	if (this->m_Timer >= this->m_GameDuration)
	{
		int winner = -1;
		for (int i = 0; i < this->m_TeamCount; i++)
		{
			if (this->GetPlace(i + 1) == 1)
			{
				winner = i + 1;
				break;
			}
		}
		this->EndGame(winner);
	}
}

void ABaseGameMode::EndGame(const int& winner)
{
	for (FConstPlayerControllerIterator itr = Super::GetWorld()->GetPlayerControllerIterator(); itr; ++itr)
	{
		AGameHUD *hud = Cast<AGameHUD>((*itr)->GetHUD());
		if (hud != nullptr)
		{
			hud->SetGameOver(winner);
		}
		(*itr)->SetIgnoreMoveInput(true);
		//(*itr)->SetInputMode(FInputModeUIOnly());
	}
	this->m_bGameOver = true;
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

		APlayerController *controller = UGameplayStatics::GetPlayerController(Super::GetWorld(), i);
		if(controller == nullptr)
		{
			controller = UGameplayStatics::CreatePlayer(Super::GetWorld(), -1, true);
		}
		if (controller == nullptr)
		{
			continue;
		}
		if (controller->GetPawn() != nullptr)
		{
			controller->GetPawn()->Destroy();
		}
		controller->Possess(character);
		controller->SetIgnoreMoveInput(true);
	}
}

void ABaseGameMode::OnBlockPlace_Implementation(ABlock *block, AActor *source)
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

void ABaseGameMode::OnBlockDestroy_Implementation(ABlock *block, AActor *source)
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

void ABaseGameMode::OnBlockPickup_Implementation(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount)
{
}

void ABaseGameMode::OnBlockDrop_Implementation(class ABlockEntity *block, class APlayerCharacter *player, const int& newCount)
{
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