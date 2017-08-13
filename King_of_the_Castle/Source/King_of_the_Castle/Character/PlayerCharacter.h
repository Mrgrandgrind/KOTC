// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

#define DEFAULT_REACH_DISTANCE (KOTC_CONSTRUCTION_REACH_MULTIPLIER * KOTC_CONSTRUCTION_REACH_DISTANCE) // The reach distance of the trace (roughly 4 blocks)

UENUM(Blueprintable)
enum class EAttackType : uint8
{
	Upper,
	Lower,
	Forward
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

	// Return the player index (e.g. 0 for player1, 1 for player2, etc..)
	int GetPlayerIndex() const;

	// Set the players team
	void SetTeam(const int32& team);

	// Are either of the brushes visible
	bool IsBrushVisible() const;

	// Set the visibility of both brushes
	void SetBrushVisible(const bool& visible) const;

	// Toggle the building mechanics
	void SetBuildModeEnabled(const bool& enable);

	// Drop the current active block from primary brush
	virtual void DropBlock();

	virtual void Dodge();

	virtual void AttackForward();

	virtual void AttackUpper();

	virtual void AttackLower();

	virtual bool CanAttack();

	virtual void CheckAttackCollision(UCapsuleComponent *capsule, const float& damageMultiplier = 1.0f);

	UFUNCTION(BlueprintNativeEvent)
	void OnRagdollBegin();
	void OnRagdollBegin_Implementation() { Super::GetController()->SetIgnoreMoveInput(true); }

	UFUNCTION(BlueprintNativeEvent)
	void OnRagdollEnd();
	void OnRagdollEnd_Implementation() { Super::GetController()->SetIgnoreMoveInput(false); }

	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerDamaged(AActor *other, const float& damage);
	void OnPlayerDamaged_Implementation(AActor *other, const float& damage) { }

	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerAttack(AActor *other, const float& damage);
	void OnPlayerAttack_Implementation(AActor *other, const float& damage) { }

	// Stun the player. You can set the duration (-1 = default) and whether or not to regenerate health to full after stun.
	UFUNCTION(BlueprintNativeEvent)
	void OnStunned(const float& duration, bool regen, bool respawn);

	//damage handling
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent,
		class AController *eventInstigator, AActor *damageCauser) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

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
	const bool& IsAttacking() const { return this->m_bAttacking; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	const EAttackType& GetAttackType() const { return this->m_AttackType; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMovementBlocked() const { return this->m_bBlockMovement || this->m_bIsStunned; }

	UCameraComponent* GetCamera();

	FORCEINLINE const int32& GetTeam() const { return this->m_Team; }

	FORCEINLINE const bool& IsBuildModeEnabled() const { return this->m_bBuildingEnabled; }

	FORCEINLINE void AddBuildArea(class ABuildArea *area) { this->m_BuildAreas.Add(area); }

	FORCEINLINE void RemoveBuildArea(class ABuildArea *area) { this->m_BuildAreas.Remove(area); }

	FORCEINLINE void SetBuildReach(const float& reach) { this->m_BuildReach = reach; }

	FORCEINLINE class UPrimaryBrush* GetPrimaryBrush() const { return this->m_PrimaryBrush; }

	FORCEINLINE class USecondaryBrush* GetSecondaryBrush() const { return this->m_SecondaryBrush; }

	FORCEINLINE void SetHealth(const float& health)
	{
		this->m_Health = FMath::Max(FMath::Min(health, this->m_MaxHealth), 0.0f);
		if (this->m_Health <= 0.0f) { this->OnStunned(this->m_StunDelay, true, true); }
	}

	FORCEINLINE void SetStamina(const float& stamina)
	{
		this->m_Stamina = FMath::Max(FMath::Min(stamina, this->m_MaxStamina), 0.0f);
	}

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

protected:
	class ABuildArea* GetActiveBuildArea();

	void TogglePause();

	void ToggleBuildMode() { if(!this->m_bBlockMovement) this->SetBuildModeEnabled(!this->m_bBuildingEnabled); }

	virtual void Attack(const EAttackType& type, const float& predelay, UCapsuleComponent *capsule, const float& damageMultiplier = 1.0f);

	//void UpdateMovementSpeed() const;

	// Checks to see if the player has req stamina. If they don't, the stamina hud will flash to indicate
	UFUNCTION(BlueprintCallable, Category = "Checks")
	bool HasStamina(const float& req);

	// Input: Show scores
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputToggleScoresTable();

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

	// Input: Rush
	//UFUNCTION(BlueprintCallable, Category = "Event")
	//void InputRushEnable() { this->m_bRushing = true; this->UpdateMovementSpeed(); }

	//UFUNCTION(BlueprintCallable, Category = "Event")
	//void InputRushDisable() { this->m_bRushing = false; this->UpdateMovementSpeed(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checks", meta = (AllowPrivateAccess = "true", DisplayName = "Sprinting"))
	bool m_bSprinting;

private:
	// Whether or not the build place input has been activated
	bool m_bPlacePressed;
	float m_PlacePressCounter;

	// Stop the player from moving
	bool m_bBlockMovement, m_bBlockAttack;

	// State booleans
	bool m_bIsStunned, m_bAttacking, m_bRushing;

	// Current attack type
	EAttackType m_AttackType;

	// Timers and counters
	float m_DamageTimer, m_DodgeCooldownCounter;

	// Current players health and stamina
	float m_Health, m_Stamina;

	UPROPERTY()
	TArray<class ABuildArea*> m_BuildAreas;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Sprint Speed"))
	float m_SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Speed"))
	float m_RushSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Turn Multiplier"))
	float m_RushTurnMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Knockback Force"))
	float m_RushKnockbackForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Knockback Offset"))
	FVector m_RushKnockbackOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Hit Stun Duration"))
	float m_RushHitStunDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Acceleration"))
	float m_RushAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Stamina Cost"))
	float m_RushStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Force"))
	float m_DodgeForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Cooldown Time"))
	float m_DodgeCooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Stamina Cost"))
	float m_DodgeStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Sprint Stamina Cost"))
	float m_SprintStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Stun Delay (seconds)"))
	float m_StunDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Speed"))
	float m_MeleeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Block Damage"))
	float m_MeleeBlockDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Player Damage"))
	float m_MeleePlayerDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Stamina Cost"))
	float m_MeleeStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Knockback Force"))
	float m_MeleeKnockbackForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Knockback Offset"))
	FVector m_MeleeKnockbackOffset;

	// Current player team. Set using #SetTeam
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int32 m_Team;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Upper Melee Capsule"))
	UCapsuleComponent *m_UpperMeleeCapsule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Lower Melee Capsule"))
	UCapsuleComponent *m_LowerMeleeCapsule;

	// Melee collision component attached to characters hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Forward Melee Capsule"))
	UCapsuleComponent *m_ForwardMeleeCapsule;

	// This players primary (create) brush
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Primary Brush"))
	class UPrimaryBrush *m_PrimaryBrush;

	// This players secondary (destroy) brush
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Secondary Brush"))
	class USecondaryBrush *m_SecondaryBrush;

	UPROPERTY()
	UMaterialInterface *m_Material;
};
