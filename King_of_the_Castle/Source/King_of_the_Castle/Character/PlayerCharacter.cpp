// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "PlayerCharacter.h"

#include "../HUD/GameHUD.h"
#include "../Construction/Block.h"
#include "../Construction/BlockData.h"
#include "../Construction/BuildArea.h"
#include "../Construction/BlockEntity.h"
#include "../Construction/Blocks/FlagBlock.h"
#include "../Construction/Brush/PrimaryBrush.h"
#include "../Construction/Brush/SecondaryBrush.h"
#include "../Gamemode/BaseGameMode.h"

#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"

#define TRACE_SOCKET TEXT("Head") // The socket (of the player) which the trace originates from
#define REACH_DISTANCE (150.0f * KOTC_CONSTRUCTION_REACH_DISTANCE) // The reach distance of the trace (roughly 4 blocks)

#define DROP_STRENGTH_MAX 750.0f
#define DROP_STRENGTH_MIN 900.0f
#define DROP_ROTATION_OFFSET 100.0f //degrees

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

	// Create the melee capsule
	this->MeleeCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeComponent"));
	this->MeleeCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	this->MeleeCapsule->SetupAttachment(RootComponent);

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

	// Update team collision (required for doors to function)
	this->SetTeam(this->m_Team);

	m_RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

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

	if (Stamina < MaxStamina) {
		if ((Stamina += m_StamRegen) > MaxStamina) {
			Stamina = MaxStamina;
		}
		else {
			Stamina += m_StamRegen;
		}
	}
	if (this->m_bPressed)
	{
		this->m_PressTimer += delta;
	}
	if (m_ChargeActive) {
		m_ChargeTimer += delta;
	}
	if (m_ChargeMove) {
					if (this->GetActorLocation() != m_ChargeMoveTo) {
						if (!this->SetActorLocation(FMath::VInterpTo(this->GetActorLocation(), m_ChargeMoveTo, delta, m_ChargeSpeed), true)) {
							ChargePunchAttack();
						}

						/*const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
						Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), m_ChargeMove);*/
					}
					else {
						ChargePunchAttack();
					}
	}
	else if (m_Dodging) {
		if (this->GetActorLocation() != m_DodgeTo) {
			if (!this->SetActorLocation(FMath::VInterpTo(this->GetActorLocation(), m_DodgeTo, delta, m_DodgeSpeed), true)) {
				m_Dodging = false;
			}
		}
		else {
			m_Dodging = false;
		}
	}
	if (m_Rushing == true) {
		if (Stamina <= 0) {
			m_Rushing = false;
			GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
		}
	}
	if (Super::GetController() != nullptr)
	{
		// Update building wheel
		AGameHUD *hud = Cast<AGameHUD>(((APlayerController*)Super::GetController())->GetHUD());
		if (hud != nullptr && hud->GetBuildWheel()->IsVisible())
		{
			float x = Super::InputComponent->GetAxisValue(TEXT("LeftThumbX")),
				y = Super::InputComponent->GetAxisValue(TEXT("LeftThumbY"));
			if(x == 0.0f && y == 0.0f)
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

UBuildWheel* APlayerCharacter::GetBuildWheel() const
{
	if(Super::GetController() == nullptr)
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
}

void APlayerCharacter::SetTeam(const int& team)
{
	this->m_Team = team;
	
	UBuildWheel *wheel = this->GetBuildWheel();
	if(wheel != nullptr)
	{
		wheel->SetTeam(team);
	}
	Super::GetCapsuleComponent()->SetCollisionProfileName(team <= 1
		? TEXT("PawnTeam1") : team >= 2 ? TEXT("PawnTeam2") : TEXT("Pawn"));
}

void APlayerCharacter::Jump()
{
	if(this->m_bBlockMovement)
	{
		return;
	}
	Super::Jump();
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

void APlayerCharacter::InputShowBuildWheel()
{
	UBuildWheel *wheel = this->GetBuildWheel();
	if(wheel != nullptr)
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
	//if (Super::Controller != nullptr && value != 0.0f && !this->m_bBlockMovement)
	//{
	//	// find out which way is forward and add the movement
	//	const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
	//	Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), value);
	//}
	if(this->m_bBlockMovement)
	{
		return;
	}
	if (!IsStunned) {
		if (Super::Controller != nullptr && value != 0.0f)
		{
			if (m_DodgeTrigger == true) {
				if (m_Dodging != true) {
					if (value > 0.5 || value < -0.5) {
						Dodge(value, 0);
					}
				}
			}
			// find out which way is forward and add the movement
			else {
				const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
				Super::AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::X), value);
				if (m_Rushing == true) {
					Stamina -= m_RushCost;
					TArray<AActor*> EnemyList;
					MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());	
					
					//for every enemy that's within the capsule, check and apply collision
					for (auto Enemies : EnemyList)
					{
						if (Enemies != this)
						{
							if (Enemies->IsA(APlayerCharacter::StaticClass()))
							{
								auto Enemy = (APlayerCharacter*)Enemies;
								if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
								{
									if (Enemy->Health > 0)
									{
										auto loc1 = Enemies->GetActorLocation();
										auto loc2 = this->GetActorLocation();
										FVector LaunchDir = (loc1 - loc2);
										FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*m_RushKnockback;
										Enemy->LaunchCharacter(Launch, 0, 0);
									}
								}
							}
						}
					}
				}

			}
		

		}
	}
}

void APlayerCharacter::MoveRight(float value)
{
	//if (Super::Controller != nullptr && value != 0.0f && !this->m_bBlockMovement)
	//{
	//	// find out which way is right and add the movement
	//	const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
	//	AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::Y), value);
	//}
	if(this->m_bBlockMovement)
	{
		return;
	}
	if (!IsStunned) {
		if (Super::Controller != nullptr && value != 0.0f)
		{
			if (m_DodgeTrigger == true) {
				if (m_Dodging != true) {
					if (value > 0.5 || value < -0.5) {
						Dodge(0, value);
					}
				}
			}
			else {
				// find out which way is right and add the movement
				const FRotator yaw(0.0f, Super::Controller->GetControlRotation().Yaw, 0.0f);
				AddMovementInput(FRotationMatrix(yaw).GetUnitAxis(EAxis::Y), value);
				if (m_Rushing == true) {
					Stamina -= m_RushCost;
						TArray<AActor*> EnemyList;
						MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());

						//for every enemy that's within the capsule, check and apply collision
						for (auto Enemies : EnemyList)
						{
							if (Enemies != this)
							{
								if (Enemies->IsA(APlayerCharacter::StaticClass()))
								{
									auto Enemy = (APlayerCharacter*)Enemies;
									if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
									{
										if (Enemy->Health > 0)
										{
											auto loc1 = Enemies->GetActorLocation();
											auto loc2 = this->GetActorLocation();
											FVector LaunchDir = (loc1 - loc2);
											FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*m_RushKnockback;
											Enemy->LaunchCharacter(Launch, 0, 0);
										}
									}
								}
							}
						}
				}
					
			}
		}
	}
}

void APlayerCharacter::DropBlock()
{
	this->DropBlock(this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetSelectedIndex()), 1);
}

void APlayerCharacter::DropBlock(UBlockData* data, int count)
{
	if(data == nullptr || count <= 0)
	{
		return;
	}
	count = FMath::Min(count, data->GetCount());
	// Drop count amount of blocks (limited to count of data - will not drop a block if it doesn't have it)
	for(int i = 0; i < count; i++)
	{
		for(ABlockEntity *next : ABlockEntity::SpawnBlockEntity((ABlock*)data->GetClassType()->GetDefaultObject(), Super::GetWorld(), nullptr, true))
		{
			next->SetBlockOwner(this);
			next->SetIgnoreOwner(true);

			next->SetActorLocation(Super::GetActorLocation());
			next->SetActorRotation(Super::GetActorRotation());
			
			FVector rotation = Super::GetActorRotation().Vector();
			((UPrimitiveComponent*)next->GetRootComponent())->AddImpulse(rotation
				* FMath::FRandRange(DROP_STRENGTH_MIN, DROP_STRENGTH_MAX));
		}
	}
	data->SetCount(this->m_PrimaryBrush, data->GetCount() - count);
}

void APlayerCharacter::MeleeAttack() 
{
	if (!IsAttacking) 
	{
		if (!IsStunned) 
		{
			AnimWeakPunch = true;
			IsAttacking = true;
			FDamageEvent ThisDamage;
			TArray<AActor*> EnemyList;
			MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());
			//for every enemy that's within the capsule, check and apply collision
			for (auto Enemies : EnemyList) 
			{
				if (Enemies != this) 
				{
					if (Enemies->IsA(APlayerCharacter::StaticClass()))
					{
						auto Enemy = (APlayerCharacter*)Enemies;
						if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent())) 
						{
							if (Enemy->Health > 0) 
							{
								Enemy->TakeDamage(MeleeAttackDamage, ThisDamage, this->GetController(), this);
								auto loc1 = Enemies->GetActorLocation();
								auto loc2 = this->GetActorLocation();
								FVector LaunchDir = (loc1 - loc2);
								FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*PlayerKnockback;
								Enemy->LaunchCharacter(Launch, 0, 0);
							}
						}
					}
					else if (Enemies->IsA(ABlock::StaticClass()) && ((ABlock*)Enemies)->IsDestructable())
					{
						Enemies->TakeDamage(this->MeleeAttackDamage * 3.0f, ThisDamage, this->GetController(), this);
					}
				}
			}
			IsAttacking = false;
		}
	}
}

void APlayerCharacter::PunchChargeUp() {
	if (!m_ChargePunch) {
		if (!IsStunned) {
			m_ChargeActive = true;
			m_ChargePunch = true;
		}
	}
}

void APlayerCharacter::ChargePunchMove() {
m_ChargeActive = false;
float movedist;
	if (m_ChargeTimer > 1) {
		if (m_ChargeTimer <= 2) {
			movedist = m_ChargeDist;
		}
		else if (m_ChargeTimer <= 3) {
			movedist = m_ChargeDist*2;
		}
		else {
			movedist = m_ChargeDist*3;
		}
	}
	else {
		movedist = 0;
	}
	if (movedist != 0) {
		m_ChargeMoveTo = this->GetActorLocation() + (this->GetActorForwardVector()*movedist);
		m_ChargeMove = true;
	}
	else {
		ChargePunchAttack();
	}
}

void APlayerCharacter::ChargePunchAttack() {
	m_ChargeMove = false;
	float damage;
	bool stun;
	float knockback;
	if (m_ChargeTimer > 1) {
		if (m_ChargeTimer <= 2) {
			damage = m_ChargeBaseDamage * 0.25;
			knockback = 0;
			stun = false;
		}
		else if (m_ChargeTimer <= 3) {
			damage = m_ChargeBaseDamage *0.5;
			knockback = m_ChargeKnockback / 2;
			stun = false;
		}
		else {
			damage = m_ChargeBaseDamage;
			knockback = m_ChargeKnockback;
			stun = true;
		}
	}
	else {
		damage = 0;
	}
	m_ChargeTimer = 0;
	if (damage != 0) {
		if (!IsAttacking)
		{
			if (!IsStunned)
			{
				IsAttacking = true;
				const FRotator yaw(0.0f, GetActorRotation().Yaw, 0.0f);
				FDamageEvent ThisDamage;
				TArray<AActor*> EnemyList;
				MeleeCapsule->GetOverlappingActors(EnemyList, TSubclassOf<APlayerCharacter>());
				//for every enemy that's within the capsule, check and apply collision
				for (auto Enemies : EnemyList)
				{
					if (Enemies != this)
					{
						if (Enemies->IsA(APlayerCharacter::StaticClass()))
						{
							auto Enemy = (APlayerCharacter*)Enemies;
							if (MeleeCapsule->IsOverlappingComponent(Enemy->GetCapsuleComponent()))
							{
								if (Enemy->Health > 0)
								{
									
									Enemy->TakeDamage(damage, ThisDamage, this->GetController(), this);
									auto loc1 = Enemies->GetActorLocation();
									auto loc2 = this->GetActorLocation();
									FVector LaunchDir = (loc1 - loc2);
									FVector Launch = (LaunchDir.GetSafeNormal() + FVector(0, 0, 0.2f))*knockback;
									Enemy->LaunchCharacter(Launch, 0, 0);
									if (stun == true) {
										Enemy->Stun(m_ChargeStun);
									}
								}
							}
						}
						else if (Enemies->IsA(ABlock::StaticClass()) && ((ABlock*)Enemies)->IsDestructable())
						{
							Enemies->TakeDamage(this->MeleeAttackDamage * 3.0f, ThisDamage, this->GetController(), this);
						}
					}
				}
				IsAttacking = false;
				m_ChargePunch = false;
			}
		}
	}
}
float APlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent,
		class AController * EventInstigator, AActor * DamageCauser) {
	if (!IsStunned) 
	{
		//apply damage, stun if 0 health, set timer to end stun
		Health -= DamageAmount;
		FTimerHandle ThisHandle;
		if (Health <= 0) 
		{
			UBlockData *data = this->m_PrimaryBrush->GetBlockData(this->m_PrimaryBrush->GetIndexOf(ID_FLAG_BLOCK));
			if(data != nullptr && data->GetCount() > 0)
			{
				this->DropBlock(data, data->GetCount());
			}

			IsStunned = true;
			GetWorldTimerManager().SetTimer(ThisHandle, this, &APlayerCharacter::EndStun, StunDuration);
		}
	}
	return DamageAmount;
}

void APlayerCharacter::Stun(float StunLength) 
{
	FTimerHandle ThisHandle;
	IsStunned = true;
	GetWorldTimerManager().SetTimer(ThisHandle, this, &APlayerCharacter::EndStun, StunLength);
}

void APlayerCharacter::EndStun() 
{
	IsStunned = false;
	Health = MaxHealth;
}

void APlayerCharacter::ToggleDodge() {
	if (m_DodgeTrigger != true) {
		m_DodgeTrigger = true;
	}
	else {
		m_DodgeTrigger = false;
	}
}

void APlayerCharacter::ToggleRush() {
	if (Stamina > 0) {
		if (m_Rushing != true) {
			m_Rushing = true;
			GetCharacterMovement()->MaxWalkSpeed = m_RushSpeed;
		}
		else {
			m_Rushing = false;
			GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
		}
	}
	else {
		m_Rushing = false;
		GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
	}
}

void APlayerCharacter::Dodge(float x, float y) 
{
	m_DodgeTo = this->GetActorLocation() + ((this->GetActorForwardVector()*m_DodgeDist)*x) +((this->GetActorRightVector()*m_DodgeDist)*y);
	m_Dodging = true;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *input)
{
	Super::SetupPlayerInputComponent(input);

	input->BindAxis("LeftThumbX");
	input->BindAxis("LeftThumbY");

	input->BindAction("Drop Block", IE_Pressed, this, &APlayerCharacter::DropBlock);

	input->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	input->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	input->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	input->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	input->BindAction("Dodge", IE_Pressed, this, &APlayerCharacter::ToggleDodge);
	input->BindAction("Dodge", IE_Released, this, &APlayerCharacter::ToggleDodge);

	input->BindAction("Rush", IE_Pressed, this, &APlayerCharacter::ToggleRush);
	input->BindAction("Rush", IE_Released, this, &APlayerCharacter::ToggleRush);

	input->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	input->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	input->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	input->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);
	
	input->BindAction("Charge Punch", IE_Pressed, this, &APlayerCharacter::PunchChargeUp);
	input->BindAction("Charge Punch", IE_Released, this, &APlayerCharacter::ChargePunchMove);

	input->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	input->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	input->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	input->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	input->BindAction("Place", IE_Pressed, this, &APlayerCharacter::InputMouseLeftDownEvent);
	input->BindAction("Place", IE_Released, this, &APlayerCharacter::InputMouseLeftUpEvent);
	input->BindAction("Destroy", IE_Pressed, this, &APlayerCharacter::InputMouseRightDownEvent);
	input->BindAction("Destroy", IE_Released, this, &APlayerCharacter::InputMouseRightUpEvent);

	input->BindAction("Build Wheel", IE_Pressed, this, &APlayerCharacter::InputShowBuildWheel);
	input->BindAction("Build Wheel", IE_Released, this, &APlayerCharacter::InputHideBuildWheel);
	input->BindAction("Build Wheel Back", IE_Pressed, this, &APlayerCharacter::InputBuildWheelBack);
	input->BindAction("Build Wheel Select", IE_Pressed, this, &APlayerCharacter::InputBuildWheelSelect);

	input->BindAction("Brush up", IE_Pressed, this, &APlayerCharacter::InputBlockTypeUpEvent);
	input->BindAction("Brush down", IE_Pressed, this, &APlayerCharacter::InputBlockTypeDownEvent);
	input->BindAction("Building toggle", IE_Pressed, this, &APlayerCharacter::ToggleBuildMode);
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