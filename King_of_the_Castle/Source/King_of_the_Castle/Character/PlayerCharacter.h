// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

#define DEFAULT_REACH_DISTANCE (KOTC_CONSTRUCTION_REACH_MULTIPLIER * KOTC_CONSTRUCTION_REACH_DISTANCE) // The reach distance of the trace (roughly 4 blocks)

UENUM(BlueprintType)		
enum class EAttackStage : uint8
{
	READY = 0			UMETA(DisplayName = "Ready"),			// Ready for attack call
	PRE_COLLISION = 1 	UMETA(DisplayName = "Pre Collision"),	// Animation has started, cannot yet apply damage
	COLLISION = 2		UMETA(DisplayName = "Collision"),		// Check for collisions and apply damage
	POST_DELAY = 3		UMETA(DisplayName = "Post Delay"),		// Attack complete, now having a delay before we become ready again
};

UCLASS()
class KING_OF_THE_CASTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float delta) override;

	// Drop the current active block from primary brush
	virtual void DropBlock();

	// Make the character jump
	void Jump() override;

	// Move the character forward and backwards
	void MoveForward(float value);

	// Move the character right and left
	void MoveRight(float value);

	// Modify camera yaw (left and right)
	void TurnAtRate(float rate);

	// Modify camera pitch (up and down)
	void LookUpAtRate(float rate);

	// Perform an attack
	void Attack();

	// Stun the player
	void Stun();

	// When the melee capsule hits something
	UFUNCTION()
	void OnMeleeEndCollision(UPrimitiveComponent *overlappedComponent, AActor *otherActor,
		UPrimitiveComponent *otherComp, int32 otherBodyIndex);

	// Return the player index (e.g. 0 for player1, 1 for player2, etc..)
	UFUNCTION(BlueprintPure, Category = "Index")
	int GetPlayerIndex() const;

	// Set the players team
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(const int& team);

	// Toggle the building mechanics
	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildModeEnabled(const bool& enable);

	//damage handling
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent,
		class AController *eventInstigator, AActor *damageCauser) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
		
	class UBuildWheel* GetBuildWheel() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	const float& GetHealth() const { return this->m_Health; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	const float& GetMaxHealth() const { return this->m_MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	const float& GetStamina() const { return this->m_Stamina; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	const float& GetMaxStamina() const { return this->m_MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	const bool& IsStunned() const { return this->m_bIsStunned; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return int(this->m_AttackStage) > 0 && this->m_AttackStage != EAttackStage::POST_DELAY;  }

	UFUNCTION(BlueprintPure, Category = "Combat")
	const EAttackStage& GetAttackStage() const { return this->m_AttackStage; }

	FORCEINLINE void SetHealth(const float& health) { this->m_Health = FMath::Min(health, this->m_MaxHealth); }

	FORCEINLINE void SetStamina(const float& stamina) { this->m_Stamina = FMath::Min(stamina, this->m_MaxStamina); }

	FORCEINLINE const int& GetTeam() const { return this->m_Team; }

	FORCEINLINE const bool& IsBuildModeEnabled() const { return this->m_bBuildingEnabled; }

	FORCEINLINE void SetBuildArea(class ABuildArea *area) { this->m_BuildArea = area; }

	FORCEINLINE void SetBuildReach(const float& reach) { this->m_BuildReach = reach; }

	FORCEINLINE class UPrimaryBrush* GetPrimaryBrush() const { return this->m_PrimaryBrush; }

	FORCEINLINE class USecondaryBrush* GetSecondaryBrush() const { return this->m_SecondaryBrush; }

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

protected:
	UFUNCTION()
	void ToggleBuildMode() { if(!this->m_bBlockMovement) this->SetBuildModeEnabled(!this->m_bBuildingEnabled); }

	// Called whenever the player moved up a block type with their brush
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockTypeUpEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockTypeDownEvent();

	// Input: Block place
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockPlaceUpEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockPlaceDownEvent();

	// Input: Block destroy
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockDestroyUpEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockDestroyDownEvent();

	// Input: Build wheel controls
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputShowBuildWheel();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputHideBuildWheel();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBuildWheelBack();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBuildWheelSelect();

private:
	// Result of last trace. The trace happens every tick when building mode is enabled.
	FHitResult m_TraceResult;

	// Whether or not the build place input has been activated
	bool m_bPlacePressed;

	// Time since build place input has been activated
	float m_PlacePressCounter;

	// Stop the player from moving
	bool m_bBlockMovement;

	// Whether or not the player is stunned
	bool m_bIsStunned;

	// Attack stage
	EAttackStage m_AttackStage;

	// Time since last attack by player. Health will not regenerate for a specified amount of time.
	float m_DamageTimer;

	// Last attacker. Only set briefly after an attack. The set player cannot hurt this player whilst set.
	APlayerCharacter *m_LastAttacker;

	// Current players health
	float m_Health;

	// Current players stamina
	float m_Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Max Health"))
	float m_MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Max Stamina"))
	float m_MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Health Regen Speed"))
	float m_HealthRegenSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Stamina Regen Speed"))
	float m_StaminaRegenSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Stun Delay (seconds)"))
	float m_StunDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Block Damage"))
	float m_MeleeBlockDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Player Damage"))
	float m_MeleePlayerDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Knockback Force"))
	float m_MeleeKnockbackForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Knockback Direction Offset"))
	FVector m_MeleeKnockbackDirOffset;

	// Current player team. Set using #SetTeam
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int m_Team;

	// Whether or not build mode is active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Build Enabled"))
	bool m_bBuildingEnabled;

	// The players build brush reach
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Build Reach"))
	float m_BuildReach;

	// Base turn rate, in deg/sec. Other scaling may affect final turn rate. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true", DisplayName = "BaseTurnRate"))
	float m_BaseTurnRate;

	// Base look up/down rate, in deg/sec. Other scaling may affect final rate. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true", DisplayName = "BaseLookUpRate"))
	float m_BaseLookUpRate;

	// Follow camera 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true", DisplayName = "Camera"))
	class UCameraComponent *m_Camera;

	// Camera boom positioning the camera behind the character 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true", DisplayName = "Camera Boom"))
	class USpringArmComponent *m_CameraBoom;

	// Melee collision component attached to characters hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Capsule"))
	UCapsuleComponent *m_MeleeCapsule;

	// Active build area where the player is standing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Build Area"))
	class ABuildArea *m_BuildArea;

	// This players primary (create) brush
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Primary Brush"))
	class UPrimaryBrush *m_PrimaryBrush;

	// This players secondary (destroy) brush
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Secondary Brush"))
	class USecondaryBrush *m_SecondaryBrush;

//public:
//	//Animation flags
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimWalk;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimSprint;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimJump;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimKnockback;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimWeakPunch;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimStrongPunch;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
//	bool AnimIdle;

////functon called to end stun
//UFUNCTION(BlueprintCallable, Category = "Combat")
//void EndStun();

//UFUNCTION(BlueprintCallable, Category = "Combat")
//void PunchChargeUp();

//UFUNCTION(BlueprintCallable, Category = "Combat")
//void ChargePunchMove();

//UFUNCTION(BlueprintCallable, Category = "Combat")
//void ChargePunchAttack();

//UFUNCTION(BlueprintCallable, Category = "Combat")
//void Stun(float StunLength);

//UFUNCTION(BlueprintCallable, Category = "Player Stats")
//void ToggleDodge();

//UFUNCTION(BlueprintCallable, Category = "Player Stats")
//void ToggleRush();

//UFUNCTION(BlueprintCallable, Category = "Player Stats")
//void Dodge(float x, float y);

//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Is Attacking?"))
//bool IsAttacking;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Is Stunned?"))
	//	bool IsStunned;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Player Knockback"))
	//float PlayerKnockback;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stamina"))
	//float Stamina;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Stamina"))
	//float MaxStamina;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Health"))
	//float Health;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Health"))
	//float MaxHealth;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Ranged Attack Cost"))
	//float RangeAttackCost;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Ranged Attack Damage"))
	//float RangeAttackDamage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Special Attack Cost"))
	//float SpecialAttackCost;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Special Attack Damage"))
	//float SpecialAttackDamage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Attack Damage"))
	//float MeleeAttackDamage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Heavy Attack Damage"))
	//float HeavyAttackDamage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stun Duration"))
	//float StunDuration;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Timer"))
	//float m_ChargeTimer;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level"))
	//int m_ChargeLevel;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 2 Knockback"))
	//float m_ChargeL2Knockback;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 3 Knockback"))
	//float m_ChargeL3Knockback;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 1 Damage"))
	//int m_ChargeL1Damage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 2 Damage"))
	//int m_ChargeL2Damage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 3 Damage"))
	//int m_ChargeL3Damage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Active"))
	//bool m_ChargeActive;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Moving"))
	//bool m_ChargeMove;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Active"))
	//bool m_ChargePunch;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Target"))
	//FVector m_ChargeMoveTo;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Speed"))
	//float m_ChargeSpeed;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Level 1 Distance"))
	//float m_ChargeL1Dist;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Level 2 Distance"))
	//float m_ChargeL2Dist;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Level 3 Distance"))
	//float m_ChargeL3Dist;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Stun Duration"))
	//float m_ChargeStun;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Toggle"))
	//bool m_DodgeTrigger;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Status"))
	//bool m_Dodging;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Start Flag"))
	//bool m_DodgeStart;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Distance"))
	//float m_DodgeDist;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Target"))
	//FVector m_DodgeTo;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Direction"))
	//FVector2D m_DodgeDir;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Speed"))
	//float m_DodgeSpeed;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Speed"))
	//float m_RushSpeed;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Status"))
	//bool m_Rushing;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Cost"))
	//float m_RushCost;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Knockback"))
	//float m_RushKnockback;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Run Speed"))
	//float m_RunSpeed;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stamina Regen"))
	//float m_StamRegen;
};
