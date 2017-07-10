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

	// Stun the player. You can set the duration (-1 = default) and whether or not to regenerate health to full after stun.
	void Stun(const float& duration = -1, const bool& regen = true);

	// When the players collision hits something
	UFUNCTION()
	void OnPlayerCollisionHit(UPrimitiveComponent *hitComponent, AActor *otherActor,
		UPrimitiveComponent *otherComp, FVector normalImpulse, const FHitResult& hit);

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

	FORCEINLINE void SetHealth(const float& health) { this->m_Health = FMath::Max(FMath::Min(health, this->m_MaxHealth), 0.0f); }

	FORCEINLINE void SetStamina(const float& stamina) { this->m_Stamina = FMath::Max(FMath::Min(stamina, this->m_MaxStamina), 0.0f); }

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

	void UpdateMovementSpeed() const;

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

	// Input: Sprint
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputSprintEnable() { this->m_bSprinting = true; this->UpdateMovementSpeed(); }

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputSprintDisable() { this->m_bSprinting = false; this->UpdateMovementSpeed(); }

	// Input: Rush
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputRushEnable() { this->m_bRushing = true; this->UpdateMovementSpeed(); }

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputRushDisable() { this->m_bRushing = false; this->UpdateMovementSpeed(); }

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

	// Whether or not the player is sprinting or rushing
	bool m_bSprinting, m_bRushing;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Health"))
	float m_MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Stamina"))
	float m_MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Health Regen Speed"))
	float m_HealthRegenSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stamina Regen Speed"))
	float m_StaminaRegenSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Walk Speed"))
	float m_WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Speed"))
	float m_RushSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Sprint Speed"))
	float m_SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Turn Multiplier"))
	float m_RushTurnMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Knockback Force"))
	float m_RushKnockbackForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Knockback Offset"))
	FVector m_RushKnockbackOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Hit Stun Duration"))
	float m_RushHitStunDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Acceleration"))
	float m_RushAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Stamina Cost"))
	float m_RushStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Sprint Stamina Cost"))
	float m_SprintStaminaCost;

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
};
