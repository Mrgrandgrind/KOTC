// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "PlayerCharacter.h"

#include "HUD/GameHUD.h"
#include "HUD/Components/CrosshairComponent.h"
#include "HUD/Components/StatBarsComponent.h"
#include "HUD/Components/ScoresOverlayComponent.h"
#include "Construction/Block.h"
#include "Construction/BlockData.h"
#include "Construction/BuildArea.h"
#include "Construction/Blocks/FlagBlock.h"
#include "Construction/Brush/PrimaryBrush.h"
#include "Construction/Brush/SecondaryBrush.h"
#include "Gamemode/BaseGameMode.h"

#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"

#define BUILD_TRACE_SOCKET TEXT("head") // The socket (of the player) which the trace originates from
#define MELEE_TRACE_SOCKET TEXT("hand_r") // The socket (of the player) where the melee collision box will be bound

#define ATTACK_UPPER_PREDELAY 0.14f
#define ATTACK_LOWER_PREDELAY 0.14f
#define ATTACK_FORWARD_PREDELAY 0.6f
#define ATTACK_DURATION 1.8f

#define CHARGE_ATTACK_ANIMATION_DURATION 2.0f

#define LAST_ATTACKER_TIMER 0.1f

#define DAMAGE_TIMER 2.0f // How long the player has to be out of combat for before health starts to regen again

#define MATERIAL_LOCATION TEXT("Material'/Game/AdvancedLocomotionV2/Characters/Mannequin/lambert2.lambert2'")

// Sets default values
APlayerCharacter::APlayerCharacter() : m_Team(-1), m_BuildReach(KOTC_CONSTRUCTION_BLOCK_REACH)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(MATERIAL_LOCATION);
	this->m_Material = Material.Object;

	this->m_MeleeSpeed = 1.0f;
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

	// Create the primary brush
	this->m_PrimaryBrush = UObject::CreateDefaultSubobject<UPrimaryBrush>(TEXT("PrimaryBrush"));
	this->m_PrimaryBrush->SetTeam(&this->m_Team);
	this->m_PrimaryBrush->SetupAttachment(Super::RootComponent);

	// Create the secondary brush
	this->m_SecondaryBrush = UObject::CreateDefaultSubobject<USecondaryBrush>(TEXT("SecondaryBrush"));
	this->m_SecondaryBrush->SetTeam(&this->m_Team);
	this->m_SecondaryBrush->SetupAttachment(Super::RootComponent);

	// Upper melee capsule
	this->m_UpperMeleeCapsule = UObject::CreateDefaultSubobject<UCapsuleComponent>(TEXT("UpperMeleeCapsule"));
	this->m_UpperMeleeCapsule->bGenerateOverlapEvents = true;
	this->m_UpperMeleeCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	this->m_UpperMeleeCapsule->SetupAttachment(Super::RootComponent);

	// Lower melee capsule
	this->m_LowerMeleeCapsule = UObject::CreateDefaultSubobject<UCapsuleComponent>(TEXT("LowerMeleeCapsule"));
	this->m_LowerMeleeCapsule->bGenerateOverlapEvents = true;
	this->m_LowerMeleeCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	this->m_LowerMeleeCapsule->SetupAttachment(Super::RootComponent);

	// Forward melee capsule
	this->m_ForwardMeleeCapsule = UObject::CreateDefaultSubobject<UCapsuleComponent>(TEXT("ForwardMeleeCapsule"));
	this->m_ForwardMeleeCapsule->bGenerateOverlapEvents = true;
	this->m_ForwardMeleeCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	this->m_ForwardMeleeCapsule->SetupAttachment(Super::RootComponent);

	Super::PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Start with full health and stamina
	this->m_Health = this->m_MaxHealth;
	this->m_Stamina = this->m_MaxStamina;
	//this->UpdateMovementSpeed();

	// For some strange reason the primary brush gets unset by unreal. This will ensure that it is set. (Otherwise the game will crash)
	if (this->m_PrimaryBrush == nullptr)
	{
		this->m_PrimaryBrush = Super::FindComponentByClass<UPrimaryBrush>();
		check(this->m_PrimaryBrush);
	}
	if (this->m_SecondaryBrush == nullptr)
	{
		this->m_SecondaryBrush = Super::FindComponentByClass<USecondaryBrush>();
		check(this->m_SecondaryBrush);
	}

	// Update team collision (required for doors to function)
	this->SetTeam(this->m_Team);
	this->SetBuildModeEnabled(true);
}

bool APlayerCharacter::HasStamina(const float& req)
{
	if (this->m_Stamina >= req)
	{
		return true;
	}
	UStatBarsComponent *component = AGameHUD::FindComponent<UStatBarsComponent>(this);
	if (component != nullptr)
	{
		component->FlashStamina();
	}
	return false;
}

UCameraComponent* APlayerCharacter::GetCamera()
{
	if (this->m_Camera != nullptr && this->m_Camera->bIsActive)
	{
		return this->m_Camera;
	}
	TInlineComponentArray<UCameraComponent*> cameras;
	Super::GetComponents<UCameraComponent>(cameras);

	for (UCameraComponent *next : cameras)
	{
		if (next->bIsActive)
		{
			this->m_Camera = next;
			break;
		}
	}
	return this->m_Camera;
}

ABuildArea* APlayerCharacter::GetActiveBuildArea()
{
	if (this->m_BuildAreas.Num() == 0)
	{
		TArray<AActor*> actors;
		Super::GetOverlappingActors(actors);

		for (AActor *next : actors)
		{
			if (next->IsA(ABuildArea::StaticClass()))
			{
				this->m_BuildAreas.Add(Cast<ABuildArea>(next));
			}
		}
	}
	return this->m_BuildAreas.Num() == 0 ? nullptr : this->m_BuildAreas.Last();
}

void APlayerCharacter::TogglePause()
{
	APlayerController *controller = (APlayerController*)Super::GetController();
	AGameHUD *hud = Cast<AGameHUD>(controller->GetHUD());
	if (hud != nullptr)
	{
		hud->SetPaused(!hud->IsPaused());
	}
	ABaseGameMode *gamemode = GetGameMode(Super::GetWorld());
	if (gamemode != nullptr)
	{
		if (hud->IsPaused())
		{
			gamemode->AddPausedPlayer(this);
		}
		else
		{
			gamemode->RemovePausedPlayer(this);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float delta)
{
	Super::Tick(delta);

	if (GetGameMode(Super::GetWorld())->IsGameOver())
	{
		if (this->IsBrushVisible())
		{
			this->SetBrushVisible(false);
		}
		return;
	}

	// Dodge timer. This will allow for double tapping A to dodge and single tap to jump.
	if (this->m_DodgeCooldownCounter <= this->m_DodgeCooldownTime)
	{
		this->m_DodgeCooldownCounter += delta;
	}

	// Damage timer. Stops the player regenerating health until out of combat for DAMAGE_TIMER seconds.
	if (this->m_DamageTimer < DAMAGE_TIMER)
	{
		this->m_DamageTimer += delta;
	}

	// Regenerate health and stamina
	if (this->m_Health < this->m_MaxHealth && this->m_Health > 0.0f && this->m_DamageTimer >= DAMAGE_TIMER)
	{
		this->SetHealth(this->m_Health + this->m_HealthRegenSpeed * delta);
	}
	if (this->m_Stamina < this->m_MaxStamina && !this->m_bRushing && !this->m_bSprinting && !this->m_bAttacking && !this->m_bIsStunned)
	{
		this->SetStamina(this->m_Stamina + this->m_StaminaRegenSpeed * delta);
	}

	if (this->IsAttacking())
	{
		Super::AddMovementInput(this->GetCamera()->GetForwardVector(), 0.1f);
	}

	if (this->m_bSprinting)
	{
		this->SetStamina(this->m_Stamina - this->m_SprintStaminaCost * delta);
	}

	// Updating building mechanics
	ABuildArea *area = this->GetActiveBuildArea();
	if (area == nullptr)
	{
		if (this->IsBrushVisible())
		{
			this->SetBrushVisible(false);
		}
	}
	else if (this->m_bBuildingEnabled)
	{
		bool canBuild = area->CanTeamBuild(this->m_Team);
		if (canBuild != this->IsBrushVisible())
		{
			this->SetBrushVisible(canBuild);
		}
		if (!canBuild)
		{
			return;
		}
		UCameraComponent *camera = this->GetCamera();
		FVector cameraLoc = camera->GetComponentLocation(), forward = camera->GetForwardVector();
		FVector cameraToPlayer = Super::GetMesh()->GetSocketLocation(BUILD_TRACE_SOCKET) - cameraLoc;

		FVector farDir = FVector(forward.X, forward.Y, 0.0f).GetSafeNormal();
		float len = FMath::Min(1.0f / FVector::DotProduct(forward, farDir), KOTC_CONSTRUCTION_BLOCK_REACH * 2.0f);

		FVector start = cameraLoc + forward * FVector::DotProduct(cameraToPlayer, forward);
		FVector end = start + forward * this->m_BuildReach * len * area->GetCellSize();

		this->m_PrimaryBrush->RenderTrace(start, start + farDir * this->m_BuildReach * area->GetCellSize(), FColor::Purple);

		// Perform the trace from the player in the direction of the camera
		FHitResult result;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		Super::GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_WorldDynamic, params);

		// Update create brush
		this->m_PrimaryBrush->Update(this, area, result);

		// Update modify brush
		this->m_SecondaryBrush->Update(this, area, result);
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

bool APlayerCharacter::IsBrushVisible() const
{
	return this->m_PrimaryBrush->IsBrushVisible() || this->m_SecondaryBrush->IsBrushVisible();
}

void APlayerCharacter::SetBrushVisible(const bool& visible)
{
	this->m_PrimaryBrush->SetBrushVisible(visible);
	this->m_SecondaryBrush->SetBrushVisible(visible);
}

void APlayerCharacter::SetBuildModeEnabled(const bool& enable)
{
	this->m_bBuildingEnabled = true;
	if (!true)
	{
		this->SetBrushVisible(true);
	}
	if (Super::GetController() != nullptr)
	{
		UCrosshairComponent *component = AGameHUD::FindComponent<UCrosshairComponent>(this);
		if (component == nullptr)
		{
			return;
		}
		component->SetVisible(true);
	}
}

void APlayerCharacter::SetTeam(const int32& team)
{
	this->m_Team = team;

	Super::GetCapsuleComponent()->SetCollisionProfileName(team <= 1
		? TEXT("PawnTeam1") : team >= 2 ? TEXT("PawnTeam2") : TEXT("Pawn"));

	this->m_PrimaryBrush->OnTeamChange(team);
	this->m_SecondaryBrush->OnTeamChange(team);

	ABaseGameMode *gamemode = GetGameMode<ABaseGameMode>(Super::GetWorld());
	if (gamemode == nullptr || this->m_Material == nullptr)
	{
		return;
	}
	UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(Super::GetMesh()->GetMaterial(0));
	if (material == nullptr)
	{
		material = UMaterialInstanceDynamic::Create(this->m_Material, Super::GetMesh()->GetOuter());
		Super::GetMesh()->SetMaterial(0, material);
	}
	material->SetVectorParameterValue(TEXT("TeamColor"), gamemode->GetTeamColor(team));
}

void APlayerCharacter::DropBlock()
{
	if (this->m_bBuildingEnabled)
	{
		this->m_PrimaryBrush->DropBlocks(this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetSelectedIndex()), 1);
	}
}

//void APlayerCharacter::UpdateMovementSpeed() const
//{
//	float speed;
//	if (this->m_bRushing)
//	{
//		// Rush speed is an acceleration process
//		speed = FMath::Min(Super::GetCharacterMovement()->MaxWalkSpeed
//			+ this->m_RushAcceleration * Super::GetWorld()->GetDeltaSeconds(), this->m_RushSpeed);
//	}
//	else if (this->m_bSprinting)
//	{
//		speed = this->m_SprintSpeed;
//	}
//	else
//	{
//		speed = this->m_WalkSpeed;
//	}
//	Super::GetCharacterMovement()->MaxWalkSpeed = speed;
//}

void APlayerCharacter::Dodge()
{
	if (!this->HasStamina(this->m_DodgeStaminaCost))
	{
		return;
	}
	// Left analogue stick input
	float x = Super::InputComponent->GetAxisValue(TEXT("LeftThumbX")),
		y = Super::InputComponent->GetAxisValue(TEXT("LeftThumbY"));

	if (x == 0.0f && y == 0.0f)
	{
		APlayerController *controller = (APlayerController*)Super::GetController();
		if (controller->IsInputKeyDown(EKeys::W))
		{
			y = 1.0f;
		}
		if (controller->IsInputKeyDown(EKeys::A))
		{
			x = -1.0f;
		}
		if (controller->IsInputKeyDown(EKeys::S))
		{
			y = -1.0f;
		}
		if (controller->IsInputKeyDown(EKeys::D))
		{
			x = 1.0f;
		}
	}

	// If there's no direction
	if (x == 0.0f && y == 0.0f)
	{
		return;
	}
	Super::GetCharacterMovement()->GroundFriction = 0.0f;

	// The way the player is facing
	FVector forward = this->GetCamera()->GetForwardVector();

	// Same as forward but as FRotator and no Z component
	FRotator rotation = FVector(forward.X, forward.Y, 0.0f).Rotation().Add(0.0f, 90.0f, 0.0f);

	// Rotate the controller input by the forward rotator and normalize for direction
	FVector direction = rotation.RotateVector(FVector(x, -y, 0.0f)).GetSafeNormal();
	Super::LaunchCharacter(direction * this->m_DodgeForce, true, true);

	// Take stamina
	this->SetStamina(this->m_Stamina - this->m_DodgeStaminaCost);

	this->m_DodgeCooldownCounter = 0.0f;

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		Super::GetCharacterMovement()->GroundFriction = 8.0f;
	}), CHARGE_ATTACK_ANIMATION_DURATION, false);
}

void APlayerCharacter::Attack()
{
	if (this->m_bAttacking || this->m_bIsStunned || GetGameMode(Super::GetWorld())->IsGameOver())
	{
		return;
	}
	if (Super::GetCharacterMovement()->IsFalling())
	{
		return;
	}
	if (!this->HasStamina(this->m_MeleeStaminaCost))
	{
		return;
	}
	this->m_bAttacking = true;
	this->m_AttackType = EAttackType::Forward;

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		this->SetStamina(this->m_Stamina - this->m_MeleeStaminaCost);
		this->CheckAttackCollision(this->m_ForwardMeleeCapsule);

		this->m_bAttacking = false;
	}), ATTACK_FORWARD_PREDELAY / this->m_MeleeSpeed, false);
}

void APlayerCharacter::AttackUpper()
{
	if (this->m_bAttacking || this->m_bIsStunned || GetGameMode(Super::GetWorld())->IsGameOver())
	{
		return;
	}
	if (!this->HasStamina(this->m_MeleeStaminaCost))
	{
		return;
	}
	this->m_bAttacking = true;
	this->m_AttackType = EAttackType::Upper;

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		this->SetStamina(this->m_Stamina - this->m_MeleeStaminaCost);
		this->CheckAttackCollision(this->m_UpperMeleeCapsule);

		this->m_bAttacking = false;
	}), ATTACK_UPPER_PREDELAY, false);
}

void APlayerCharacter::AttackLower()
{
	if (this->m_bAttacking || this->m_bIsStunned || GetGameMode(Super::GetWorld())->IsGameOver())
	{
		return;
	}
	if (!Super::GetCharacterMovement()->IsFalling())
	{
		return;
	}
	if (!this->HasStamina(this->m_MeleeStaminaCost))
	{
		return;
	}
	FVector origin, extent;
	Super::GetActorBounds(true, origin, extent);

	FVector start = Super::GetActorLocation() - FVector(0.0f, 0.0f, extent.Z / 2.0f),
		end = start - FVector(0.0f, 0.0f, 500.0f);

	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	FHitResult result;
	Super::GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_WorldDynamic, params);

	if (!result.IsValidBlockingHit())
	{
		this->Attack();
		return;
	}

	this->m_bAttacking = true;
	this->m_AttackType = EAttackType::Lower;

	float height = Super::GetActorLocation().Z;
	Super::LaunchCharacter(FVector(0.0f, 0.0f, -800.0f), false, true);

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		this->SetStamina(this->m_Stamina - this->m_MeleeStaminaCost);
		this->CheckAttackCollision(this->m_LowerMeleeCapsule);

		this->m_bAttacking = false;
	}), ATTACK_LOWER_PREDELAY, false);
}

void APlayerCharacter::CheckAttackCollision(UCapsuleComponent *capsule, const float& damageMultiplier)
{
	if (capsule == nullptr)
	{
		return;
	}
	TArray<AActor*> overlapping;
	capsule->GetOverlappingActors(overlapping);

	for (AActor *next : overlapping)
	{
		if (next == this)
		{
			continue;
		}
		FDamageEvent event;
		if (next->IsA(ABlock::StaticClass()))
		{
			next->TakeDamage(this->m_MeleeBlockDamage * damageMultiplier, event, Super::GetController(), this);
		}
		if (next->IsA(APlayerCharacter::StaticClass()))
		{
			APlayerCharacter *player = Cast<APlayerCharacter>(next);

			// Do not hurt this player if they are on the same team
			if (player->GetTeam() == this->GetTeam())
			{
				continue;
			}
			// Apply damage to player if they haven't recently been hit
			player->TakeDamage(this->m_MeleePlayerDamage * damageMultiplier, event, Super::GetController(), this);

			// Apply knockback force
			FVector direction = (player->GetActorLocation() - Super::GetActorLocation()).GetSafeNormal();
			player->LaunchCharacter((direction + this->m_MeleeKnockbackOffset)
				* this->m_MeleeKnockbackForce, false, false);

			// Reset damage timer so they don't regenerate health for a few seconds
			player->m_DamageTimer = 0.0f;
		}
	}
}

void APlayerCharacter::OnStunned_Implementation(const float& duration, bool regen, bool respawn)
{
	// Don't stun if already stunned
	if (this->m_bIsStunned)
	{
		return;
	}
	this->m_bIsStunned = true;
	Super::GetController()->SetIgnoreMoveInput(true);

	// Automatically drop the flag block (if carrying) when stunned
	UBlockData *data = this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetIndexOf(ID_FLAG_BLOCK));
	if (data != nullptr && data->GetCount() > 0)
	{
		this->m_PrimaryBrush->DropBlocks(data, data->GetCount());
	}

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([&]()
	{
		if (regen)
		{
			this->m_Health = this->m_MaxHealth;
			this->m_Stamina = this->m_MaxStamina;
		}
		if (respawn)
		{
			FVector location;
			FRotator rotation;

			ABaseGameMode *gamemode = GetGameMode(Super::GetWorld());
			if (gamemode != nullptr && gamemode->GetSpawnPoint(this->GetTeam(), location, rotation))
			{
				Super::SetActorLocation(location);
				Super::SetActorRotation(rotation);
			}
		}
		this->m_bIsStunned = false;
		Super::GetController()->SetIgnoreMoveInput(false);
	}), duration <= 0.0f ? this->m_StunDelay : duration, false);
}

void APlayerCharacter::OnAttacked_Implementation(AActor *other, const float& damage)
{
}

float APlayerCharacter::TakeDamage(float damageAmount, FDamageEvent const& damageEvent,
	AController *eventInstigator, AActor *damageCauser)
{
	if (!this->m_bIsStunned)
	{
		this->OnAttacked(damageCauser, damageAmount);
		this->SetHealth(this->m_Health - damageAmount);
	}
	return damageAmount;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *input)
{
	Super::SetupPlayerInputComponent(input);

	input->BindAxis("LeftThumbX");
	input->BindAxis("LeftThumbY");

	//input->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	//input->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//input->BindAction("Rush", IE_Pressed, this, &APlayerCharacter::InputRushEnable);
	//input->BindAction("Rush", IE_Released, this, &APlayerCharacter::InputRushDisable);

	input->BindAction("Dodge", IE_Pressed, this, &APlayerCharacter::Dodge);
	input->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);
	input->BindAction("Attack High", IE_Pressed, this, &APlayerCharacter::AttackUpper);
	input->BindAction("Attack Low", IE_Pressed, this, &APlayerCharacter::AttackLower);

	input->BindAction("Place Block", IE_Pressed, this, &APlayerCharacter::InputBlockPlaceDownEvent);
	input->BindAction("Place Block", IE_Released, this, &APlayerCharacter::InputBlockPlaceUpEvent);
	input->BindAction("Destroy Block", IE_Pressed, this, &APlayerCharacter::InputBlockDestroyDownEvent);
	input->BindAction("Destroy Block", IE_Released, this, &APlayerCharacter::InputBlockDestroyUpEvent);

	//input->BindAction("Brush up", IE_Pressed, this, &APlayerCharacter::InputBlockTypeUpEvent);
	//input->BindAction("Brush down", IE_Pressed, this, &APlayerCharacter::InputBlockTypeDownEvent);
	//input->BindAction("Building toggle", IE_Pressed, this, &APlayerCharacter::ToggleBuildMode);

	input->BindAction("Drop Block", IE_Pressed, this, &APlayerCharacter::DropBlock);

	input->BindAction("View Scores", IE_Pressed, this, &APlayerCharacter::InputToggleScoresTable);
	input->BindAction("View Scores", IE_Released, this, &APlayerCharacter::InputToggleScoresTable);

	//input->BindAction("Pause", IE_Pressed, this, &APlayerCharacter::TogglePause);
}

void APlayerCharacter::InputToggleScoresTable()
{
	UScoresOverlayComponent *component = AGameHUD::FindComponent<UScoresOverlayComponent>(this);
	if (component == nullptr)
	{
		return;
	}
	component->SetVisible(!component->IsVisible());
}

void APlayerCharacter::InputBlockTypeUpEvent()
{
	if (this->m_bBuildingEnabled)
	{
		this->m_PrimaryBrush->SetSelectedIndex(this->m_PrimaryBrush->GetSelectedIndex() - 1);
	}
}

void APlayerCharacter::InputBlockTypeDownEvent()
{
	if (this->m_bBuildingEnabled)
	{
		this->m_PrimaryBrush->SetSelectedIndex(this->m_PrimaryBrush->GetSelectedIndex() + 1);
	}
}

void APlayerCharacter::InputBlockPlaceDownEvent()
{
	this->m_bPlacePressed = true;
	this->m_PlacePressCounter = 0.0f;

	if (this->m_PrimaryBrush->IsBrushVisible())
	{
		this->m_PrimaryBrush->SetChainMode(true);
	}
}

void APlayerCharacter::InputBlockPlaceUpEvent()
{
	this->m_bPlacePressed = false;
	this->m_PlacePressCounter = 0.0f;

	ABuildArea *area = this->GetActiveBuildArea();
	if (area != nullptr && this->m_bBuildingEnabled && area->CanTeamBuild(this->m_Team))
	{
		this->m_PrimaryBrush->Action(area, this);
	}
	this->m_PrimaryBrush->SetChainMode(false);
}

void APlayerCharacter::InputBlockDestroyDownEvent()
{
	if (this->m_SecondaryBrush->IsBrushVisible())
	{
		this->m_SecondaryBrush->SetChainMode(true);
	}
}

void APlayerCharacter::InputBlockDestroyUpEvent()
{
	ABuildArea *area = this->GetActiveBuildArea();
	if (area != nullptr && this->m_bBuildingEnabled && area->CanTeamBuild(this->m_Team))
	{
		this->m_SecondaryBrush->Action(area, this);
	}
	this->m_SecondaryBrush->SetChainMode(false);
}

#if WITH_EDITOR
void APlayerCharacter::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(APlayerCharacter, m_Team))
	{
		this->SetTeam(this->m_Team);
	}
}
#endif