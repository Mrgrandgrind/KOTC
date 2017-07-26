// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "PlayerCharacter.h"

#include "HUD/GameHUD.h"
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

#define ATTACK_PRE_DELAY 0.6f 
#define ATTACK_COLLISION_DELAY 0.4f
#define ATTACK_POST_DELAY 0.2f

#define CHARGE_ATTACK_ANIMATION_DURATION 2.0f

#define LAST_ATTACKER_TIMER 0.1f

#define DAMAGE_TIMER 2.0f // How long the player has to be out of combat for before health starts to regen again
#define DODGE_DOUBLE_TAP_TIME 0.32f

#define MATERIAL_LOCATION TEXT("Material'/Game/AdvancedLocomotionV2/Characters/Mannequin/lambert2.lambert2'")

// Sets default values
APlayerCharacter::APlayerCharacter() : m_bPlacePressed(false), m_PlacePressCounter(0.0f), m_AttackStage(EAttackStage::READY),
m_DodgePressTimer(DODGE_DOUBLE_TAP_TIME), m_Team(-1), m_BuildReach(KOTC_CONSTRUCTION_BLOCK_REACH)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(MATERIAL_LOCATION);
	this->m_Material = Material.Object;

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

	// Create melee collision capsule
	this->m_MeleeCapsule = UObject::CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeCapsule"));
	this->m_MeleeCapsule->bGenerateOverlapEvents = true;
	this->m_MeleeCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//this->m_MeleeCapsule->SetupAttachment(Super::GetMesh(), MELEE_TRACE_SOCKET);
	this->m_MeleeCapsule->SetupAttachment(Super::RootComponent);

	// Delegate the melee capsule collisions
	TScriptDelegate<FWeakObjectPtr> delegateMelee;
	delegateMelee.BindUFunction(this, FName("OnMeleeEndCollision"));
	this->m_MeleeCapsule->OnComponentEndOverlap.Add(delegateMelee);

	// Enable hit events on the capsule
	TScriptDelegate<FWeakObjectPtr> delegateCap;
	delegateCap.BindUFunction(this, FName("OnPlayerCollisionHit"));
	Super::GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	Super::GetCapsuleComponent()->OnComponentHit.Add(delegateCap);

	Super::PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Start with full health and stamina
	this->m_Health = this->m_MaxHealth;
	this->m_Stamina = this->m_MaxStamina;
	this->UpdateMovementSpeed();

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
}

UCameraComponent* APlayerCharacter::GetCamera()
{
	if (this->m_Camera != nullptr)
	{
		//return this->m_Camera;
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

// Called every frame
void APlayerCharacter::Tick(float delta)
{
	Super::Tick(delta);

	// Dodge timer. This will allow for double tapping A to dodge and single tap to jump.
	if (this->m_DodgePressTimer < DODGE_DOUBLE_TAP_TIME)
	{
		this->m_DodgePressTimer += delta;
	}
	if (this->m_DodgeCooldownCounter <= this->m_DodgeCooldownTime)
	{
		this->m_DodgeCooldownCounter += delta;
	}

	// Charger counter. To check how long the player has been charging their attack for.
	if (this->m_bCharging)
	{
		this->m_ChargeCounter = FMath::Min(this->m_ChargeCounter + delta, this->m_ChargeDuration);
	}

	// Damage timer. Stops the player regenerating health until out of combat for DAMAGE_TIMER seconds.
	if (this->m_DamageTimer < DAMAGE_TIMER)
	{
		this->m_DamageTimer += delta;
	}

	// Regenerate health and stamina
	if (this->m_Health < this->m_MaxHealth && this->m_DamageTimer >= DAMAGE_TIMER)
	{
		this->SetHealth(this->m_Health + this->m_HealthRegenSpeed * delta);
	}
	if (this->m_Stamina < this->m_MaxStamina && !this->m_bRushing && !this->m_bSprinting)
	{
		this->SetStamina(this->m_Stamina + this->m_StaminaRegenSpeed * delta);
	}

	// Subtract cost of stamina if rushing or sprinting
	if (this->m_bRushing)
	{
		this->UpdateMovementSpeed();

		this->SetStamina(this->m_Stamina - this->m_RushStaminaCost * delta);
		if (this->m_Stamina == 0.0f)
		{
			this->InputRushDisable();
		}
	}
	else if (this->m_bSprinting)
	{
		this->SetStamina(this->m_Stamina - this->m_SprintStaminaCost * delta);
		if (this->m_Stamina == 0.0f)
		{
			this->InputSprintDisable();
		}
	}

	// Update building wheel
	//if (Super::GetController() != nullptr)
	//{
	//	AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
	//	if (hud != nullptr && hud->GetBuildWheel()->IsVisible())
	//	{
	//		float x = Super::InputComponent->GetAxisValue(TEXT("LeftThumbX")),
	//			y = Super::InputComponent->GetAxisValue(TEXT("LeftThumbY"));
	//		if (x == 0.0f && y == 0.0f)
	//		{
	//			hud->GetBuildWheel()->SetSelected(-1);
	//		}
	//		else
	//		{
	//			hud->GetBuildWheel()->SetSelected(FMath::Atan2(y, x));
	//		}
	//	}
	//}

	// Updating building mechanics
	ABuildArea *area = this->GetActiveBuildArea();
	if (area == nullptr || Super::GetCharacterMovement()->IsFalling())
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

//UBuildWheel* APlayerCharacter::GetBuildWheel() const
//{
//	if (Super::GetController() == nullptr)
//	{
//		return nullptr;
//	}
//	AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
//	return hud == nullptr ? nullptr : hud->GetBuildWheel();
//}

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
	this->m_bBuildingEnabled = enable;
	if (!enable)
	{
		this->SetBrushVisible(enable);
	}
	if (Super::GetController() != nullptr)
	{
		AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
		if (hud != nullptr)
		{
			hud->SetCrosshairVisible(enable);
		}
	}
}

void APlayerCharacter::SetTeam(const int32& team)
{
	this->m_Team = team;

	//UBuildWheel *wheel = this->GetBuildWheel();
	//if (wheel != nullptr)
	//{
	//	wheel->SetTeam(team);
	//}
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

void APlayerCharacter::UpdateMovementSpeed() const
{
	float speed;
	if (this->m_bRushing)
	{
		// Rush speed is an acceleration process
		speed = FMath::Min(Super::GetCharacterMovement()->MaxWalkSpeed
			+ this->m_RushAcceleration * Super::GetWorld()->GetDeltaSeconds(), this->m_RushSpeed);
	}
	else if (this->m_bSprinting)
	{
		speed = this->m_SprintSpeed;
	}
	else
	{
		speed = this->m_WalkSpeed;
	}
	Super::GetCharacterMovement()->MaxWalkSpeed = speed;
}

void APlayerCharacter::Dodge()
{
	if (this->m_Stamina < this->m_DodgeStaminaCost)
	{
		return;
	}
	// Left analogue stick input
	float x = Super::InputComponent->GetAxisValue(TEXT("LeftThumbX")),
		y = Super::InputComponent->GetAxisValue(TEXT("LeftThumbY"));

	// If there's no direction do nothing
	if(x == 0.0f && y == 0.0f)
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
		this->m_bCharging = false;
		Super::GetCharacterMovement()->GroundFriction = 8.0f;
	}), CHARGE_ATTACK_ANIMATION_DURATION, false);
}

void APlayerCharacter::Attack()
{
	if (this->m_bIsStunned || this->m_AttackStage != EAttackStage::READY)
	{
		return;
	}
	if (this->m_Stamina < this->m_MeleeStaminaCost)
	{
		return;
	}
	this->m_AttackStage = EAttackStage::PRE_COLLISION;

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		TArray<AActor*> overlapping;
		this->m_MeleeCapsule->GetOverlappingActors(overlapping);

		for (AActor *next : overlapping)
		{
			if (next == this)
			{
				continue;
			}
			FDamageEvent event;
			if (next->IsA(ABlock::StaticClass()))
			{
				next->TakeDamage(this->m_MeleeBlockDamage, event, Super::GetController(), this);
			}
			if (next->IsA(APlayerCharacter::StaticClass()))
			{
				APlayerCharacter *player = Cast<APlayerCharacter>(next);

				// Do not hurt this player if they have immunity or are on the same team
				if (player->m_LastAttacker == this || player->GetTeam() == this->GetTeam())
				{
					return;
				}
				// Apply damage to player if they haven't recently been hit
				player->TakeDamage(this->m_MeleePlayerDamage, event, Super::GetController(), this);

				// Apply knockback force
				FVector direction = (player->GetActorLocation() - Super::GetActorLocation()).GetSafeNormal();
				player->LaunchCharacter((direction + this->m_MeleeKnockbackOffset)
					* this->m_MeleeKnockbackForce, false, false);

				// Stop the player from being damaged multiple times by the same collision by granting temporary immunity
				player->m_LastAttacker = this;
				player->m_DamageTimer = 0.0f; // Reset damage timer so they don't regenerate health for a few seconds

				FTimerHandle handle2;
				Super::GetWorldTimerManager().SetTimer(handle2, FTimerDelegate::CreateLambda([this, player]()
				{
					player->m_LastAttacker = nullptr;
				}), LAST_ATTACKER_TIMER, false);
			}
		}

		this->m_AttackStage = EAttackStage::READY;
	}), ATTACK_PRE_DELAY, false);

	//FTimerHandle handle1;
	//Super::GetWorldTimerManager().SetTimer(handle1, FTimerDelegate::CreateLambda([this]()
	//{
	//	FTimerHandle handle2;
	//	this->m_AttackStage = EAttackStage::COLLISION;
	//	Super::GetWorldTimerManager().SetTimer(handle2, FTimerDelegate::CreateLambda([this]()
	//	{
	//		FTimerHandle handle3;
	//		this->m_AttackStage = EAttackStage::POST_DELAY;
	//		Super::GetWorldTimerManager().SetTimer(handle3, FTimerDelegate::CreateLambda([this]()
	//		{
	//			this->m_AttackStage = EAttackStage::READY;
	//		}), ATTACK_POST_DELAY, false);
	//	}), ATTACK_COLLISION_DELAY, false);
	//}), ATTACK_PRE_DELAY, false);
}

void APlayerCharacter::Stun(const float& duration, const bool& regen, const bool& respawn)
{
	// Don't stun if already stunned
	if (this->m_bIsStunned)
	{
		return;
	}
	this->m_bIsStunned = true;

	// Automatically drop the flag block (if carrying) when stunned
	UBlockData *data = this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetIndexOf(ID_FLAG_BLOCK));
	if (data != nullptr && data->GetCount() > 0)
	{
		this->m_PrimaryBrush->DropBlocks(data, data->GetCount());
	}

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this, regen, respawn]()
	{
		this->m_bIsStunned = false;
		if (regen)
		{
			this->m_Health = this->m_MaxHealth;
		}
		if (respawn)
		{
			FVector location;
			FRotator rotation;

			ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
			if (gamemode != nullptr && gamemode->GetSpawnPoint(this->GetTeam(), location, rotation))
			{
				Super::SetActorLocation(location);
				Super::SetActorRotation(rotation);
			}
		}
	}), duration <= 0.0f ? this->m_StunDelay : duration, false);
}

void APlayerCharacter::InputChargeDisable()
{
	Super::GetCharacterMovement()->GroundFriction = 0.0f;

	FVector forward = this->GetCamera()->GetForwardVector();
	forward.Z = 0.0f;
	forward.Normalize();

	float power = this->m_ChargeDuration == 0.0f ? 1.0f : this->m_ChargeCounter / this->m_ChargeDuration;
	Super::LaunchCharacter(forward * this->m_ChargeSlideForce * power, true, true);

	//this->Attack();

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		this->m_bCharging = false;
		Super::GetCharacterMovement()->GroundFriction = 8.0f;
	}), CHARGE_ATTACK_ANIMATION_DURATION, false);
}

void APlayerCharacter::OnPlayerCollisionHit(UPrimitiveComponent* hitComponent, AActor* otherActor,
	UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hit)
{
	// Check to see if we hit another player that's not on the same team
	APlayerCharacter *player = Cast<APlayerCharacter>(otherActor);
	if (player != nullptr && player->GetTeam() != this->GetTeam())
	{
		FVector direction = (player->GetActorLocation() - Super::GetActorLocation()).GetSafeNormal();
		if (this->m_bCharging)
		{
			// Get charge power percentage
			float power = this->m_ChargeDuration == 0.0f ? 1.0f : this->m_ChargeCounter / this->m_ChargeDuration;

			// Knockback this player in the opposite direction
			direction += this->m_ChargeKnockbackOffset;
			player->LaunchCharacter(direction * this->m_ChargeKnockbackForce * power, true, true);

			// Temporarily stun the opponent
			player->Stun(this->m_ChargeStunDuration * power);
		}
		else if (this->m_bRushing)
		{
			// Get rush power percentage
			float power = this->m_RushSpeed == 0.0f ? 0.0f : (Super::GetCharacterMovement()->MaxWalkSpeed / this->m_RushSpeed);

			// Knockback the enemy player
			direction += this->m_MeleeKnockbackOffset;
			player->LaunchCharacter(direction * this->m_RushKnockbackForce * power, true, true);

			// Knockback this player in the opposite direction
			Super::LaunchCharacter(-direction * this->m_RushKnockbackForce * power, true, true);

			// Temporarily stun the opponent
			player->Stun(this->m_RushHitStunDuration * power);

			// Drain all stamina
			this->SetStamina(0.0f);

			// Disable rush
			this->InputRushDisable();
		}
	}
}

void APlayerCharacter::OnMeleeEndCollision(UPrimitiveComponent* overlappedComponent,
	AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	// Ignore collisions if we are not currently attacking
	if (this->m_AttackStage != EAttackStage::COLLISION || otherActor == nullptr || otherActor == this)
	{
		return;
	}
	FDamageEvent event;
	if (otherActor->IsA(ABlock::StaticClass()))
	{
		otherActor->TakeDamage(this->m_MeleeBlockDamage, event, Super::GetController(), this);
	}
	if (otherActor->IsA(APlayerCharacter::StaticClass()))
	{
		APlayerCharacter *player = Cast<APlayerCharacter>(otherActor);

		// Do not hurt this player if they have immunity or are on the same team
		if (player->m_LastAttacker == this || player->GetTeam() == this->GetTeam())
		{
			return;
		}
		// Apply damage to player if they haven't recently been hit
		player->TakeDamage(this->m_MeleePlayerDamage, event, Super::GetController(), this);

		// Apply knockback force
		FVector direction = (player->GetActorLocation() - Super::GetActorLocation()).GetSafeNormal();
		player->LaunchCharacter((direction + this->m_MeleeKnockbackOffset)
			* this->m_MeleeKnockbackForce, false, false);

		// Stop the player from being damaged multiple times by the same collision by granting temporary immunity
		player->m_LastAttacker = this;
		player->m_DamageTimer = 0.0f; // Reset damage timer so they don't regenerate health for a few seconds

		FTimerHandle handle;
		Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this, player]()
		{
			player->m_LastAttacker = nullptr;
		}), LAST_ATTACKER_TIMER, false);
	}
}

float APlayerCharacter::TakeDamage(float damageAmount, FDamageEvent const& damageEvent,
	AController *eventInstigator, AActor *damageCauser)
{
	if (!this->m_bIsStunned)
	{
		this->SetHealth(this->m_Health - damageAmount);
		if (this->m_Health <= 0.0f)
		{
			this->Stun();
		}
	}
	return damageAmount;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *input)
{
	Super::SetupPlayerInputComponent(input);

	input->BindAxis("LeftThumbX");
	input->BindAxis("LeftThumbY");

	//input->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	//input->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);

	//input->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//input->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	//input->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//input->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	//input->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//input->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	//input->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//input->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	input->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	input->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	input->BindAction("Rush", IE_Pressed, this, &APlayerCharacter::InputRushEnable);
	input->BindAction("Rush", IE_Released, this, &APlayerCharacter::InputRushDisable);

	input->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::InputSprintEnable);
	input->BindAction("Sprint", IE_Released, this, &APlayerCharacter::InputSprintDisable);

	input->BindAction("Dodge", IE_Pressed, this, &APlayerCharacter::Dodge);
	input->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);

	//input->BindAction("Charge Attack", IE_Pressed, this, &APlayerCharacter::InputChargeEnable);
	//input->BindAction("Charge Attack", IE_Released, this, &APlayerCharacter::InputChargeDisable);

	input->BindAction("Place Block", IE_Pressed, this, &APlayerCharacter::InputBlockPlaceDownEvent);
	input->BindAction("Place Block", IE_Released, this, &APlayerCharacter::InputBlockPlaceUpEvent);
	input->BindAction("Destroy Block", IE_Pressed, this, &APlayerCharacter::InputBlockDestroyDownEvent);
	input->BindAction("Destroy Block", IE_Released, this, &APlayerCharacter::InputBlockDestroyUpEvent);

	//input->BindAction("Build Wheel", IE_Pressed, this, &APlayerCharacter::InputShowBuildWheel);
	//input->BindAction("Build Wheel", IE_Released, this, &APlayerCharacter::InputHideBuildWheel);
	//input->BindAction("Build Wheel Back", IE_Pressed, this, &APlayerCharacter::InputBuildWheelBack);
	//input->BindAction("Build Wheel Select", IE_Pressed, this, &APlayerCharacter::InputBuildWheelSelect);

	input->BindAction("Brush up", IE_Pressed, this, &APlayerCharacter::InputBlockTypeUpEvent);
	input->BindAction("Brush down", IE_Pressed, this, &APlayerCharacter::InputBlockTypeDownEvent);
	input->BindAction("Building toggle", IE_Pressed, this, &APlayerCharacter::ToggleBuildMode);

	input->BindAction("Drop Block", IE_Pressed, this, &APlayerCharacter::DropBlock);
}

void APlayerCharacter::Jump()
{
	if (!this->m_bBlockMovement)
	{
		Super::Jump();

		//if (this->m_DodgePressTimer < DODGE_DOUBLE_TAP_TIME 
		//	&& this->m_DodgeCooldownCounter >= this->m_DodgeCooldownTime)
		//{
		//	this->Dodge();
		//	Super::StopJumping();

		//	this->m_DodgePressTimer = DODGE_DOUBLE_TAP_TIME;
		//}
		//else
		//{
		//	this->m_DodgePressTimer = 0.0f;
		//}
	}
}

void APlayerCharacter::TurnAtRate(float rate)
{
	// If rush is active multiply the turn amount by a specified multiplier
	if (this->m_bRushing)
	{
		rate *= this->m_RushTurnMultiplier;
	}
	Super::AddControllerYawInput(rate * this->m_BaseTurnRate * Super::GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float rate)
{
	Super::AddControllerPitchInput(rate * this->m_BaseLookUpRate * Super::GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float value)
{
	if (this->m_bBlockMovement || this->m_bIsStunned)
	{
		return;
	}
	if (Super::Controller != nullptr && value != 0.0f)
	{
		// Find out which way is forward and add the movement
		const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
		Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), value);
	}
}

void APlayerCharacter::MoveRight(float value)
{
	if (this->m_bBlockMovement || this->m_bIsStunned)
	{
		return;
	}
	if (Super::Controller != nullptr && value != 0.0f)
	{
		// If rush is active multiply the turn amount by a specified multiplier
		if (this->m_bRushing)
		{
			value *= this->m_RushTurnMultiplier;
		}

		// Find out which way is right and add the movement
		const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
		Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::Y), value);
	}
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

//void APlayerCharacter::InputShowBuildWheel()
//{
//	UBuildWheel *wheel = this->GetBuildWheel();
//	if (wheel != nullptr)
//	{
//		wheel->SetVisible(true);
//	}
//	this->m_bBlockMovement = true;
//}
//
//void APlayerCharacter::InputHideBuildWheel()
//{
//	UBuildWheel *wheel = this->GetBuildWheel();
//	if (wheel != nullptr)
//	{
//		wheel->SetVisible(false);
//	}
//	this->m_bBlockMovement = false;
//}
//
//void APlayerCharacter::InputBuildWheelBack()
//{
//	UBuildWheel *wheel = this->GetBuildWheel();
//	if (wheel != nullptr && wheel->IsVisible())
//	{
//		wheel->Back();
//	}
//}
//
//void APlayerCharacter::InputBuildWheelSelect()
//{
//	UBuildWheel *wheel = this->GetBuildWheel();
//	if (wheel != nullptr && wheel->IsVisible())
//	{
//		wheel->Select(this);
//	}
//}

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