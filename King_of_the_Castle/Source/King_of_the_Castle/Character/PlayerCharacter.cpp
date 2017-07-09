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

#define BUILD_TRACE_SOCKET TEXT("bind_head01") // The socket (of the player) which the trace originates from
#define BUILD_TRACE_SOCKET TEXT("bind_r_lowerarm01") // The socket (of the player) where the melee collision box will be bound

#define ATTACK_PRE_DELAY 0.4f 
#define ATTACK_COLLISION_DELAY 0.4f
#define ATTACK_POST_DELAY 0.2f

#define LAST_ATTACKER_TIMER 0.1f

#define DAMAGE_TIMER 2.0f // How long the player has to be out of combat for before health starts to regen again

// Sets default values
APlayerCharacter::APlayerCharacter() : m_bPlacePressed(false), m_PlacePressCounter(0.0f),
m_AttackStage(EAttackStage::READY), m_Team(-1), m_BuildReach(DEFAULT_REACH_DISTANCE)
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
	this->m_CameraBoom->TargetArmLength = 300.0f;
	this->m_CameraBoom->bUsePawnControlRotation = true;
	this->m_CameraBoom->SetupAttachment(Super::RootComponent);

	// Create a follow camera
	this->m_Camera = UObject::CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	this->m_Camera->bUsePawnControlRotation = false;
	this->m_Camera->SetupAttachment(this->m_CameraBoom, USpringArmComponent::SocketName);

	// Create the primary brush
	this->m_PrimaryBrush = UObject::CreateDefaultSubobject<UPrimaryBrush>(TEXT("PrimaryBrush"));
	this->m_PrimaryBrush->SetTeam(&this->m_Team);
	this->m_PrimaryBrush->SetupAttachment(Super::RootComponent);

	// Create the secondary brush
	this->m_SecondaryBrush = UObject::CreateDefaultSubobject<USecondaryBrush>(TEXT("SecondaryBrush"));
	this->m_SecondaryBrush->SetTeam(&this->m_Team);
	this->m_SecondaryBrush->SetCraftTimer(&this->m_PlacePressCounter);
	this->m_SecondaryBrush->SetupAttachment(Super::RootComponent);

	// Create melee collision capsule
	this->m_MeleeCapsule = UObject::CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeCapsule"));
	this->m_MeleeCapsule->bGenerateOverlapEvents = true;
	this->m_MeleeCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	this->m_MeleeCapsule->SetupAttachment(Super::GetMesh(), BUILD_TRACE_SOCKET);

	// Delegate the melee capsule collisions
	TScriptDelegate<FWeakObjectPtr> delegate;
	delegate.BindUFunction(this, FName("OnMeleeEndCollision"));
	this->m_MeleeCapsule->OnComponentEndOverlap.Add(delegate);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Start with full health and stamina
	this->m_Health = this->m_MaxHealth;
	this->m_Stamina = this->m_MaxStamina;

	// Update team collision (required for doors to function)
	this->SetTeam(this->m_Team);

	//m_RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

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

	// Connect construction and door block data counts
	//UBlockData *construction = this->m_PrimaryBrush->GetBlockData(EBlockType::Construction);
	//UBlockData *door = this->m_PrimaryBrush->GetBlockData(EBlockType::Door);
	//if (construction != nullptr && door != nullptr)
	//{
	//	door->JoinCount(construction);
	//}
}

// Called every frame
void APlayerCharacter::Tick(float delta)
{
	Super::Tick(delta);

	// Damage timer. Stops the player regenerating health until out of combat for DAMAGE_TIMER seconds.
	if(this->m_DamageTimer < DAMAGE_TIMER)
	{
		this->m_DamageTimer += delta;
	}

	// Regenerate health
	if(this->m_Health < this->m_MaxHealth && this->m_DamageTimer >= DAMAGE_TIMER)
	{
		this->SetHealth(this->m_Health + this->m_HealthRegenSpeed * delta);
	}
	// Regenerate stamina
	if (this->m_Stamina < this->m_MaxStamina)
	{
		this->SetStamina(this->m_Stamina + this->m_StaminaRegenSpeed * delta);
	}

	if (Super::GetController() != nullptr)
	{
		// Update building wheel
		AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
		if (hud != nullptr && hud->GetBuildWheel()->IsVisible())
		{
			float x = Super::InputComponent->GetAxisValue(TEXT("LeftThumbX")),
				y = Super::InputComponent->GetAxisValue(TEXT("LeftThumbY"));
			if (x == 0.0f && y == 0.0f)
			{
				hud->GetBuildWheel()->SetSelected(-1);
			}
			else
			{
				hud->GetBuildWheel()->SetSelected(FMath::Atan2(y, x));
			}
		}
	}
	if (this->m_BuildArea != nullptr && this->m_bBuildingEnabled && this->m_BuildArea->GetTeam() == this->m_Team)
	{
		FVector cameraLoc = this->m_Camera->GetComponentLocation(), forward = this->m_Camera->GetForwardVector();
		FVector cameraToPlayer = Super::GetMesh()->GetSocketLocation(BUILD_TRACE_SOCKET) - cameraLoc;

		FVector start = cameraLoc + forward * FVector::DotProduct(cameraToPlayer, forward);
		FVector end = start + forward * (/*cameraToPlayer.Size() + */this->m_BuildReach);

		// Perform the trace from the player in the direction of the camera
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		Super::GetWorld()->LineTraceSingleByChannel(this->m_TraceResult, start, end, ECollisionChannel::ECC_WorldDynamic, params);

		// Update create brush
		this->m_PrimaryBrush->Update(this, this->m_BuildArea, this->m_TraceResult);

		// Update modify brush
		this->m_SecondaryBrush->Update(this, this->m_BuildArea, this->m_TraceResult);
	}
}

UBuildWheel* APlayerCharacter::GetBuildWheel() const
{
	if (Super::GetController() == nullptr)
	{
		return nullptr;
	}
	AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
	return hud == nullptr ? nullptr : hud->GetBuildWheel();
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
	AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
	if (hud != nullptr)
	{
		hud->SetCrosshairVisible(enable);
	}
}

void APlayerCharacter::SetTeam(const int& team)
{
	this->m_Team = team;

	UBuildWheel *wheel = this->GetBuildWheel();
	if (wheel != nullptr)
	{
		wheel->SetTeam(team);
	}
	Super::GetCapsuleComponent()->SetCollisionProfileName(team <= 1
		? TEXT("PawnTeam1") : team >= 2 ? TEXT("PawnTeam2") : TEXT("Pawn"));
}

void APlayerCharacter::DropBlock()
{
	if (this->m_bBuildingEnabled)
	{
		this->m_PrimaryBrush->DropBlocks(this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetSelectedIndex()), 1);
	}
}

void APlayerCharacter::Attack()
{
	if (this->m_bIsStunned || this->m_AttackStage != EAttackStage::READY)
	{
		return;
	}
	this->m_AttackStage = EAttackStage::PRE_COLLISION;

	FTimerHandle handle;
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this, &handle]()
	{
		this->m_AttackStage = EAttackStage::COLLISION;
		Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this, &handle]()
		{
			this->m_AttackStage = EAttackStage::POST_DELAY;
			Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
			{
				this->m_AttackStage = EAttackStage::READY;
			}), ATTACK_POST_DELAY, false);
		}), ATTACK_COLLISION_DELAY, false);
	}), ATTACK_PRE_DELAY, false);
}

void APlayerCharacter::Stun()
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
	Super::GetWorldTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([this]()
	{
		this->m_bIsStunned = false;
		this->m_Health = this->m_MaxHealth;
	}), this->m_StunDelay, false);
}

void APlayerCharacter::OnMeleeEndCollision(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
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

		// Do not hurt this player if they have immunity
		if(player->m_LastAttacker == this)
		{
			return;
		}
		// Apply damage to player if they haven't recently been hit
		player->TakeDamage(this->m_MeleePlayerDamage, event, Super::GetController(), this);

		// Apply knockback force
		FVector direction = (player->GetActorLocation() - Super::GetActorLocation()).GetSafeNormal();
		player->LaunchCharacter((direction + this->m_MeleeKnockbackDirOffset)
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
		this->m_Health = FMath::Max(this->m_Health - damageAmount, 0.0f);
		if (this->m_Health <= 0.0f)
		{
			this->Stun();
		}
	}
	//if (!IsStunned)
	//{
	//	//apply damage, stun if 0 health, set timer to end stun
	//	Health -= DamageAmount;
	//	FTimerHandle ThisHandle;
	//	if (Health <= 0)
	//	{
	//		UBlockData *data = this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetIndexOf(ID_FLAG_BLOCK));
	//		if (data != nullptr && data->GetCount() > 0)
	//		{
	//			this->DropBlock(data, data->GetCount());
	//		}

	//		IsStunned = true;
	//		GetWorldTimerManager().SetTimer(ThisHandle, this, &APlayerCharacter::EndStun, StunDuration);
	//	}
	//}
	return damageAmount;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *input)
{
	Super::SetupPlayerInputComponent(input);

	input->BindAxis("LeftThumbX");
	input->BindAxis("LeftThumbY");

	input->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	input->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);

	input->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	input->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	input->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	input->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	input->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	input->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	input->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	input->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	input->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	input->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	input->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);

	input->BindAction("Place Block", IE_Pressed, this, &APlayerCharacter::InputBlockPlaceDownEvent);
	input->BindAction("Place Block", IE_Released, this, &APlayerCharacter::InputBlockPlaceUpEvent);
	input->BindAction("Destroy Block", IE_Pressed, this, &APlayerCharacter::InputBlockDestroyDownEvent);
	input->BindAction("Destroy Block", IE_Released, this, &APlayerCharacter::InputBlockDestroyUpEvent);

	input->BindAction("Build Wheel", IE_Pressed, this, &APlayerCharacter::InputShowBuildWheel);
	input->BindAction("Build Wheel", IE_Released, this, &APlayerCharacter::InputHideBuildWheel);
	input->BindAction("Build Wheel Back", IE_Pressed, this, &APlayerCharacter::InputBuildWheelBack);
	input->BindAction("Build Wheel Select", IE_Pressed, this, &APlayerCharacter::InputBuildWheelSelect);

	input->BindAction("Brush up", IE_Pressed, this, &APlayerCharacter::InputBlockTypeUpEvent);
	input->BindAction("Brush down", IE_Pressed, this, &APlayerCharacter::InputBlockTypeDownEvent);
	input->BindAction("Building toggle", IE_Pressed, this, &APlayerCharacter::ToggleBuildMode);

	// For testing. TODO Ask if this should be a feature in the game
	input->BindAction("Drop Block", IE_Pressed, this, &APlayerCharacter::DropBlock);

	//input->BindAction("Dodge", IE_Pressed, this, &APlayerCharacter::ToggleDodge);
	//input->BindAction("Dodge", IE_Released, this, &APlayerCharacter::ToggleDodge);

	//input->BindAction("Rush", IE_Pressed, this, &APlayerCharacter::ToggleRush);
	//input->BindAction("Rush", IE_Released, this, &APlayerCharacter::ToggleRush);

	//input->BindAction("Charge Punch", IE_Pressed, this, &APlayerCharacter::PunchChargeUp);
	//input->BindAction("Charge Punch", IE_Released, this, &APlayerCharacter::ChargePunchMove);
}

void APlayerCharacter::Jump()
{
	if (!this->m_bBlockMovement)
	{
		Super::Jump();
	}
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
	if(this->m_bBlockMovement || this->m_bIsStunned)
	{
		return;
	}
	if (Super::Controller != nullptr && value != 0.0f)
	{
		// find out which way is forward and add the movement
		const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
		Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), value);
	}
}

void APlayerCharacter::MoveRight(float value)
{
	if(this->m_bBlockMovement || this->m_bIsStunned)
	{
		return;
	}
	if (Super::Controller != nullptr && value != 0.0f)
	{
		// find out which way is right and add the movement
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

void APlayerCharacter::InputBlockDestroyDownEvent()
{
	if (this->m_SecondaryBrush->IsBrushVisible())
	{
		this->m_SecondaryBrush->SetChainMode(true);
	}
}

void APlayerCharacter::InputBlockDestroyUpEvent()
{
	if (this->m_BuildArea != nullptr && this->m_bBuildingEnabled && this->m_BuildArea->GetTeam() == this->m_Team)
	{
		this->m_SecondaryBrush->Action(this->m_BuildArea, this);
	}
	this->m_SecondaryBrush->SetChainMode(false);
}

void APlayerCharacter::InputShowBuildWheel()
{
	UBuildWheel *wheel = this->GetBuildWheel();
	if (wheel != nullptr)
	{
		wheel->SetVisible(true);
	}
	this->m_bBlockMovement = true;
}

void APlayerCharacter::InputHideBuildWheel()
{
	UBuildWheel *wheel = this->GetBuildWheel();
	if (wheel != nullptr)
	{
		wheel->SetVisible(false);
	}
	this->m_bBlockMovement = false;
}

void APlayerCharacter::InputBuildWheelBack()
{
	UBuildWheel *wheel = this->GetBuildWheel();
	if (wheel != nullptr && wheel->IsVisible())
	{
		wheel->Back();
	}
}

void APlayerCharacter::InputBuildWheelSelect()
{
	UBuildWheel *wheel = this->GetBuildWheel();
	if (wheel != nullptr && wheel->IsVisible())
	{
		wheel->Select(this);
	}
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






//if (Stamina < MaxStamina)
//{
//	if ((Stamina += m_StamRegen) > MaxStamina)
//	{
//		Stamina = MaxStamina;
//	}
//	else
//	{
//		Stamina += m_StamRegen *  GetWorld()->GetDeltaSeconds();
//	}
//}
//if (this->m_bPressed)
//{
//	this->m_PressTimer += delta;
//}
//if (m_ChargeActive)
//{
//	m_ChargeTimer += delta;
//	if (m_ChargeTimer >= 1)
//	{
//		if (m_ChargeLevel != 3)
//		{
//			m_ChargeLevel++;
//			m_ChargeTimer = 0;
//		}
//	}
//}
//if (m_ChargeMove)
//{
//	if (this->GetActorLocation() != m_ChargeMoveTo)
//	{
//		if (!this->SetActorLocation(FMath::VInterpTo(this->GetActorLocation(), m_ChargeMoveTo, delta, m_ChargeSpeed), true))
//		{
//			ChargePunchAttack();
//		}

//		/*const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
//		Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), m_ChargeMove);*/
//	}
//	else
//	{
//		ChargePunchAttack();
//	}
//}
//else if (m_DodgeStart)
//{
//	m_DodgeTo = this->GetActorLocation() + ((this->GetActorForwardVector()*m_DodgeDist)*m_DodgeDir.X) + ((this->GetActorRightVector()*m_DodgeDist)*m_DodgeDir.Y);
//	m_Dodging = true;
//	m_DodgeStart = false;
//	m_DodgeDir.X = 0;
//	m_DodgeDir.Y = 0;
//}
//else if (m_Dodging)
//{
//	if (this->GetActorLocation() != m_DodgeTo)
//	{
//		FVector loc = this->GetActorLocation();
//		if (!this->SetActorLocation(FMath::VInterpTo(loc, m_DodgeTo, delta, m_DodgeSpeed), true))
//		{
//			m_Dodging = false;
//		}
//	}
//	else
//	{
//		m_Dodging = false;
//	}
//	m_DodgeDir.X = 0;
//	m_DodgeDir.Y = 0;
//}
//if (m_Rushing == true)
//{
//	if (Stamina <= 0)
//	{
//		m_Rushing = false;
//		GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
//	}
//}







//if (this->m_bBlockMovement)
//{
//	return;
//}
//if (!IsStunned)
//{
//	if (Super::Controller != nullptr && value != 0.0f)
//	{
//		m_DodgeDir.X = value;
//		if (m_DodgeTrigger == true)
//		{
//			if (m_Dodging != true)
//			{
//				if (value > 0.5 || value < -0.5)
//				{
//					//Dodge(value, 0);

//					m_DodgeStart = true;
//				}
//			}
//		}
//		// find out which way is forward and add the movement
//		else
//		{
//			if (!m_ChargeMove)
//			{
//				const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
//				Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), value);
//				if (m_Rushing == true)
//				{
//					Stamina -= m_RushCost;
//					TArray<AActor*> EnemyList;
//					MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());

//					//for every enemy that's within the capsule, check and apply collision
//					for (auto Enemies : EnemyList)
//					{
//						if (Enemies != this)
//						{
//							if (Enemies->IsA(APlayerCharacter::StaticClass()))
//							{
//								auto Enemy = (APlayerCharacter*)Enemies;
//								if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
//								{
//									if (Enemy->Health > 0)
//									{
//										auto loc1 = Enemies->GetActorLocation();
//										auto loc2 = this->GetActorLocation();
//										FVector LaunchDir = (loc1 - loc2);
//										FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*m_RushKnockback;
//										Enemy->LaunchCharacter(Launch, 0, 0);
//									}
//								}
//							}
//						}
//					}
//				}

//			}
//		}

//	}
//}







//if (this->m_bBlockMovement)
//{
//	return;
//}
//if (!IsStunned)
//{
//	if (Super::Controller != nullptr && value != 0.0f)
//	{
//		m_DodgeDir.Y = value;
//		if (m_DodgeTrigger == true)
//		{
//			if (m_Dodging != true)
//			{
//				if (value > 0.5 || value < -0.5)
//				{
//					//Dodge(0, value);

//					m_DodgeStart = true;
//				}
//			}
//		}
//		else
//		{
//			// find out which way is right and add the movement
//			if (!m_ChargeMove)
//			{
//				const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
//				AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::Y), value);
//				if (m_Rushing == true)
//				{
//					Stamina -= m_RushCost;
//					TArray<AActor*> EnemyList;
//					MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());

//					//for every enemy that's within the capsule, check and apply collision
//					for (auto Enemies : EnemyList)
//					{
//						if (Enemies != this)
//						{
//							if (Enemies->IsA(APlayerCharacter::StaticClass()))
//							{
//								auto Enemy = (APlayerCharacter*)Enemies;
//								if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
//								{
//									if (Enemy->Health > 0)
//									{
//										auto loc1 = Enemies->GetActorLocation();
//										auto loc2 = this->GetActorLocation();
//										FVector LaunchDir = (loc1 - loc2);
//										FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*m_RushKnockback;
//										Enemy->LaunchCharacter(Launch, 0, 0);
//									}
//								}
//							}
//						}
//					}
//				}

//			}
//		}
//	}
//}












//void APlayerCharacter::Stun(float StunLength)
//{
//FTimerHandle ThisHandle;
//IsStunned = true;
//GetWorldTimerManager().SetTimer(ThisHandle, this, &APlayerCharacter::EndStun, StunLength);
//}

//void APlayerCharacter::EndStun()
//{
//IsStunned = false;
//Health = MaxHealth;
//}

//void APlayerCharacter::ToggleDodge()
//{
//if (m_DodgeTrigger != true)
//{
//	m_DodgeTrigger = true;
//}
//else
//{
//	m_DodgeTrigger = false;
//}
//}

//void APlayerCharacter::ToggleRush()
//{
//if (Stamina > 0)
//{
//	if (m_Rushing != true)
//	{
//		m_Rushing = true;
//		GetCharacterMovement()->MaxWalkSpeed = m_RushSpeed;
//	}
//	else
//	{
//		m_Rushing = false;
//		GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
//	}
//}
//else
//{
//	m_Rushing = false;
//	GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
//}
//}

//void APlayerCharacter::Dodge(float x, float y)
//{
//m_DodgeTo = this->GetActorLocation() + ((this->GetActorForwardVector()*m_DodgeDist)*x) + ((this->GetActorRightVector()*m_DodgeDist)*y);
//m_Dodging = true;
//}

//void APlayerCharacter::MeleeAttack()
//{
//	if (!IsAttacking)
//	{
//		if (!IsStunned)
//		{
////			AnimWeakPunch = true;
//			IsAttacking = true;
//			FDamageEvent ThisDamage;
//			TArray<AActor*> EnemyList;
//			MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());
//			//for every enemy that's within the capsule, check and apply collision
//			for (auto Enemies : EnemyList)
//			{
//				if (Enemies != this)
//				{
//					if (Enemies->IsA(APlayerCharacter::StaticClass()))
//					{
//						auto Enemy = (APlayerCharacter*)Enemies;
//						if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
//						{
//							if (Enemy->Health > 0)
//							{
//								Enemy->TakeDamage(MeleeAttackDamage, ThisDamage, this->GetController(), this);
//								auto loc1 = Enemies->GetActorLocation();
//								auto loc2 = this->GetActorLocation();
//								FVector LaunchDir = (loc1 - loc2);
//								FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*PlayerKnockback;
//								Enemy->LaunchCharacter(Launch, 0, 0);
//							}
//						}
//					}
//					else if (Enemies->IsA(ABlock::StaticClass()) && ((ABlock*)Enemies)->IsDestructable())
//					{
//						Enemies->TakeDamage(this->MeleeAttackDamage * 3.0f, ThisDamage, this->GetController(), this);
//					}
//				}
//			}
//			IsAttacking = false;
//		}
//	}
//}

//void APlayerCharacter::PunchChargeUp()
//{
//if (!m_ChargePunch)
//{
//	if (!IsStunned)
//	{
//		m_ChargeActive = true;
//		m_ChargePunch = true;
//	}
//}
//}

//void APlayerCharacter::ChargePunchMove()
//{
//m_ChargeActive = false;
//float movedist;
//if (m_ChargeLevel >= 1)
//{
//	if (m_ChargeLevel < 2)
//	{
//		movedist = m_ChargeL1Dist;
//	}
//	else if (m_ChargeLevel < 3)
//	{
//		movedist = m_ChargeL2Dist;
//	}
//	else
//	{
//		movedist = m_ChargeL3Dist;
//	}
//}
//else
//{
//	movedist = 0;
//}
//if (movedist != 0)
//{
//	m_ChargeMoveTo = this->GetActorLocation() + (this->GetActorForwardVector()*movedist);
//	m_ChargeMove = true;
//}
//else
//{
//	ChargePunchAttack();
//}
//}

//void APlayerCharacter::ChargePunchAttack()
//{
//m_ChargeMove = false;
//float damage;
//bool stun;
//float knockback;
//if (m_ChargeLevel >= 1)
//{
//	if (m_ChargeLevel < 2)
//	{
//		damage = m_ChargeL1Damage;
//		knockback = 0;
//		stun = false;
//	}
//	else if (m_ChargeLevel < 3)
//	{
//		damage = m_ChargeL2Damage;
//		knockback = m_ChargeL2Knockback;
//		stun = false;
//	}
//	else
//	{
//		damage = m_ChargeL3Damage;
//		knockback = m_ChargeL3Knockback;
//		stun = true;
//	}
//}
//else
//{
//	damage = 0;
//}
//m_ChargeLevel = 0;
//m_ChargeTimer = 0;
//if (damage != 0)
//{
//	if (!IsAttacking)
//	{
//		if (!IsStunned)
//		{
//			IsAttacking = true;
//			const FRotator yaw(0.0f, GetActorRotation().Yaw, 0.0f);
//			FDamageEvent ThisDamage;
//			TArray<AActor*> EnemyList;
//			MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());
//			//for every enemy that's within the capsule, check and apply collision
//			for (auto Enemies : EnemyList)
//			{
//				if (Enemies != this)
//				{
//					if (Enemies->IsA(APlayerCharacter::StaticClass()))
//					{
//						auto Enemy = (APlayerCharacter*)Enemies;
//						if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
//						{
//							if (Enemy->Health > 0)
//							{

//								Enemy->TakeDamage(damage, ThisDamage, this->GetController(), this);
//								auto loc1 = Enemies->GetActorLocation();
//								auto loc2 = this->GetActorLocation();
//								FVector LaunchDir = (loc1 - loc2);
//								FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*knockback;
//								Enemy->LaunchCharacter(Launch, 0, 0);
//								if (stun == true)
//								{
//									Enemy->Stun(m_ChargeStun);
//								}
//							}
//						}
//					}
//					else if (Enemies->IsA(ABlock::StaticClass()) && ((ABlock*)Enemies)->IsDestructable())
//					{
//						Enemies->TakeDamage(this->MeleeAttackDamage * 3.0f, ThisDamage, this->GetController(), this);
//					}
//				}
//			}
//			IsAttacking = false;
//			m_ChargePunch = false;
//		}
//	}
//}
//}