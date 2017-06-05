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

	this->m_BaseTurnRate = 45.0f;
	this->m_BaseLookUpRate = 45.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	Super::bUseControllerRotationPitch = false;
	Super::bUseControllerRotationYaw = false;
	Super::bUseControllerRotationRoll = false;

	// Configure character movement
	Super::GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	Super::GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	Super::GetCharacterMovement()->JumpZVelocity = 600.f;
	Super::GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	this->m_CameraBoom = UObject::CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	this->m_CameraBoom->SetupAttachment(Super::RootComponent);
	this->m_CameraBoom->TargetArmLength = 300.0f; 	
	this->m_CameraBoom->bUsePawnControlRotation = true; 

	// Create a follow camera
	this->m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	this->m_Camera->SetupAttachment(this->m_CameraBoom, USpringArmComponent::SocketName);
	this->m_Camera->bUsePawnControlRotation = false;

	// Create the primary brush
	this->m_PrimaryBrush = UObject::CreateDefaultSubobject<UPrimaryBrush>(TEXT("PrimaryBrush"));
	this->m_PrimaryBrush->SetTeam(&this->m_Team);
	this->m_PrimaryBrush->SetupAttachment(Super::RootComponent);

	// Create the secondary brush
	this->m_SecondaryBrush = UObject::CreateDefaultSubobject<USecondaryBrush>(TEXT("SecondaryBrush"));
	this->m_SecondaryBrush->SetTeam(&this->m_Team);
	this->m_SecondaryBrush->SetCraftTimer(&this->m_PressTimer);
	this->m_SecondaryBrush->SetupAttachment(Super::RootComponent);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// For some strange reason the primary brush gets unset by unreal. This will ensure that it is set. (Otherwise the game will crash)
	if(this->m_PrimaryBrush == nullptr)
	{
		this->m_PrimaryBrush = Super::FindComponentByClass<UPrimaryBrush>();
		check(this->m_PrimaryBrush);
	}
	if(this->m_SecondaryBrush == nullptr)
	{
		this->m_SecondaryBrush = Super::FindComponentByClass<USecondaryBrush>();
		check(this->m_SecondaryBrush);
	}

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
		if(this->m_PrimaryBrush != nullptr)
		{
			this->m_PrimaryBrush->Update(this, this->m_BuildArea, this->m_TraceResult);
		}
		// Update modify brush
		if(this->m_SecondaryBrush != nullptr)
		{
			this->m_SecondaryBrush->Update(this, this->m_BuildArea, this->m_TraceResult);
		}

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


void APlayerCharacter::TurnAtRate(float rate)
{
	Super::AddControllerYawInput(rate * this->m_BaseTurnRate * Super::GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float rate)
{
	Super::AddControllerPitchInput(rate * this->m_BaseLookUpRate * Super::GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float value)
{
	if (Super::Controller != nullptr && value != 0.0f)
	{
		// find out which way is forward and add the movement
		const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
		Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), value);
	}
}

void APlayerCharacter::MoveRight(float value)
{
	if (Super::Controller != nullptr && value != 0.0f)
	{
		// find out which way is right and add the movement
		const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
		AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::Y), value);
	}
}

void APlayerCharacter::MeleeAttack() 
{

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	InputComponent->BindAction("Place", IE_Pressed, this, &APlayerCharacter::InputMouseLeftDownEvent);
	InputComponent->BindAction("Place", IE_Released, this, &APlayerCharacter::InputMouseLeftUpEvent);
	InputComponent->BindAction("Destroy", IE_Pressed, this, &APlayerCharacter::InputMouseRightDownEvent);
	InputComponent->BindAction("Destroy", IE_Released, this, &APlayerCharacter::InputMouseRightUpEvent);

	InputComponent->BindAction("Brush up", IE_Pressed, this, &APlayerCharacter::InputBlockTypeUpEvent);
	InputComponent->BindAction("Brush down", IE_Pressed, this, &APlayerCharacter::InputBlockTypeDownEvent);
	InputComponent->BindAction("Building toggle", IE_Pressed, this, &APlayerCharacter::ToggleBuildMode);
}