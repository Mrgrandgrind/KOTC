// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "PlayerCharacter.h"

#include "../Construction/Block.h"
#include "../Construction/BlockData.h"
#include "../Construction/BuildArea.h"
#include "../Construction/Brush/PrimaryBrush.h"
#include "../Construction/Brush/SecondaryBrush.h"
#include "../Gamemode/BaseGameMode.h"

#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"

#define TRACE_SOCKET TEXT("Head") // The socket (of the player) which the trace originates from
#define REACH_DISTANCE (150.0f * KOTC_CONSTRUCTION_REACH_DISTANCE) // The reach distance of the trace (roughly 4 blocks)

// Sets default values
APlayerCharacter::APlayerCharacter() : m_Team(-1), m_PressTimer(0.0f), m_bPressed(false)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	Super::PrimaryActorTick.bCanEverTick = true;

	this->m_PrimaryBrush = UObject::CreateDefaultSubobject<UPrimaryBrush>(TEXT("PrimaryBrush"));
	this->m_PrimaryBrush->SetTeam(&this->m_Team);
	this->m_PrimaryBrush->SetupAttachment(Super::RootComponent);

	this->m_SecondaryBrush = UObject::CreateDefaultSubobject<USecondaryBrush>(TEXT("SecondaryBrush"));
	this->m_SecondaryBrush->SetTeam(&this->m_Team);
	this->m_SecondaryBrush->SetCraftTimer(&this->m_PressTimer);
	this->m_SecondaryBrush->SetupAttachment(Super::RootComponent);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Connect construction and door block data counts
	//UBlockData *construction = this->m_PrimaryBrush->GetBlockData(EBlockType::Construction);
	//UBlockData *door = this->m_PrimaryBrush->GetBlockData(EBlockType::Door);
	//if (construction != nullptr && door != nullptr)
	//{
	//	door->JoinCount(construction);
	//}

	//if (Super::GetWorld() == nullptr)
	//{
	//	return;
	//}
	//// Set skin according to player index
	//ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
	//if (gamemode != nullptr)
	//{
	//	const TArray<UMaterialInterface*> *materials = gamemode->GetSkins(this->GetDefaultClass());
	//	if (materials != nullptr)
	//	{
	//		int index = this->GetPlayerIndex() % materials->Num();
	//		Super::GetMesh()->SetMaterial(0, (*materials)[index]);
	//	}
	//}

	// Since the camera component is created within the blueprint, we will need to find it so we can use it
	//this->m_Camera = Super::FindComponentByClass<UCameraComponent>();
	//checkf(this->m_Camera != nullptr, TEXT("The player has no camera component?"));
}

// Called every frame
void APlayerCharacter::Tick(float delta)
{
	Super::Tick(delta);

	if (this->m_bPressed)
	{
		this->m_PressTimer += delta;
	}

	if (this->m_BuildArea != nullptr && this->m_bBuildingEnabled && this->m_BuildArea->GetTeam() == this->m_Team)
	{
		// TODO For some reason we cannot get the camera component in BeginPlay. Look into why.
		this->m_Camera = Super::FindComponentByClass<UCameraComponent>();

		FVector cameraLoc = this->m_Camera->GetComponentLocation(), forward = this->m_Camera->GetForwardVector();
		FVector cameraToPlayer = Super::GetMesh()->GetSocketLocation(TRACE_SOCKET) - cameraLoc;

		FVector start = cameraLoc + forward * FVector::DotProduct(cameraToPlayer, forward);
		FVector end = start + forward * (/*cameraToPlayer.Size() + */REACH_DISTANCE);
#if KOTC_CONSTRUCTION_INFINITE_REACH
		end += forward * 100000000000.0f; //extend the reach to a massive distance (not actually infinite)
#endif

										  // Perform the trace from the player in the direction of the camera
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		Super::GetWorld()->LineTraceSingleByChannel(this->m_TraceResult, start, end, ECollisionChannel::ECC_WorldDynamic, params);

		// Update create brush
		this->m_PrimaryBrush->Update(this, this->m_BuildArea, this->m_TraceResult);

		// Update modify brush
		this->m_SecondaryBrush->Update(this, this->m_BuildArea, this->m_TraceResult);

#if WITH_EDITOR
		// If we want to draw the line trace (for debugging purposes)
		if (this->m_bDebugDrawTrace)
		{
			if (this->m_TraceResult.GetComponent() == nullptr)
			{
				DrawDebugLine(Super::GetWorld(), start, end, FColor::Red, false, -1.0f, 0.0f, 2.0f);
			}
			else
			{
				DrawDebugLine(Super::GetWorld(), start, this->m_TraceResult.ImpactPoint, FColor::Green, false, -1.0f, 0.0f, 2.0f);
				DrawDebugLine(Super::GetWorld(), this->m_TraceResult.ImpactPoint, end, FColor::Red, false, -1.0f, 0.0f, 2.0f);
			}
		}
#endif
	}
}

int APlayerCharacter::GetPlayerIndex() const
{
	ULocalPlayer *player = Cast<ULocalPlayer>(((APlayerController*)Super::GetController())->Player);
	if (player != nullptr)
	{
		return player->GetOuterUEngine()->GetGamePlayers(Super::GetWorld()).Find(player);
	}
	return -1;
}

void APlayerCharacter::SetBuildModeEnabled(const bool& enable)
{
	this->m_bBuildingEnabled = enable;
	if (!enable)
	{
		this->m_PrimaryBrush->SetBrushVisible(enable);
		this->m_SecondaryBrush->SetBrushVisible(enable);
	}
}

void APlayerCharacter::SetTeam(const int& team)
{
	this->m_Team = team;

	Super::GetCapsuleComponent()->SetCollisionProfileName(team == 1
		? TEXT("PawnTeam1") : team == 2 ? TEXT("PawnTeam2") : TEXT("Pawn"));
}

void APlayerCharacter::InputBlockTypeUpEvent()
{
	this->m_PrimaryBrush->SetSelectedIndex(this->m_PrimaryBrush->GetSelectedIndex() - 1);
}

void APlayerCharacter::InputBlockTypeDownEvent()
{
	this->m_PrimaryBrush->SetSelectedIndex(this->m_PrimaryBrush->GetSelectedIndex() + 1);
}

void APlayerCharacter::InputMouseLeftDownEvent()
{
	this->m_bPressed = true;
	this->m_PressTimer = 0.0f;

	if (this->m_PrimaryBrush->IsBrushVisible())
	{
		this->m_PrimaryBrush->SetChainMode(true);
	}
}

void APlayerCharacter::InputMouseLeftUpEvent()
{
	this->m_bPressed = false;
	this->m_PressTimer = 0.0f;
	if (this->m_BuildArea != nullptr && this->m_bBuildingEnabled && this->m_BuildArea->GetTeam() == this->m_Team)
	{
		// Check to see if the player was combining a block and then stopped
		if (!this->m_SecondaryBrush->IsCombining())
		{
			this->m_PrimaryBrush->Action(this->m_BuildArea, this);
		}
		this->m_SecondaryBrush->ReleaseCombineLock();
	}
	this->m_PrimaryBrush->SetChainMode(false);
}

void APlayerCharacter::InputMouseRightDownEvent()
{
	if (this->m_SecondaryBrush->IsBrushVisible())
	{
		this->m_SecondaryBrush->SetChainMode(true);
	}
}

void APlayerCharacter::InputMouseRightUpEvent()
{
	if (this->m_BuildArea != nullptr && this->m_bBuildingEnabled && this->m_BuildArea->GetTeam() == this->m_Team)
	{
		this->m_SecondaryBrush->Action(this->m_BuildArea, this);
	}
	this->m_SecondaryBrush->SetChainMode(false);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAction("Place", IE_Pressed, this, &APlayerCharacter::InputMouseLeftDownEvent);
	InputComponent->BindAction("Place", IE_Released, this, &APlayerCharacter::InputMouseLeftUpEvent);
	InputComponent->BindAction("Destroy", IE_Pressed, this, &APlayerCharacter::InputMouseRightDownEvent);
	InputComponent->BindAction("Destroy", IE_Released, this, &APlayerCharacter::InputMouseRightUpEvent);

	InputComponent->BindAction("Brush up", IE_Pressed, this, &APlayerCharacter::InputBlockTypeUpEvent);
	InputComponent->BindAction("Brush down", IE_Pressed, this, &APlayerCharacter::InputBlockTypeDownEvent);
	InputComponent->BindAction("Building toggle", IE_Pressed, this, &APlayerCharacter::ToggleBuildMode);
}