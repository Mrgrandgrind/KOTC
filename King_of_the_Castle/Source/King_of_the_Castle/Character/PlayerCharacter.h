// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

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

	// Drop x amount of blocks from primary brush
	virtual void DropBlock();

	// Drop x amount of blocks from primary brush
	virtual void DropBlock(class UBlockData *data, int count);

	// Return the player index (e.g. 0 for player1, 1 for player2, etc..)
	UFUNCTION(BlueprintPure, Category = "Index")
	int GetPlayerIndex() const;

	UFUNCTION()
	void MoveForward(float value);

	UFUNCTION()
	void MoveRight(float value);

	UFUNCTION()
	void TurnAtRate(float rate);

	UFUNCTION()
	void LookUpAtRate(float rate);

	UFUNCTION(BlueprintCallable, Category = "Event")
	void Jump() override;

	// Called whenever the player moved up a block type with their brush
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockTypeUpEvent();

	// Called whenever the player moves down a block type with their brush
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockTypeDownEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputMouseLeftDownEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputMouseLeftUpEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputMouseRightDownEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputMouseRightUpEvent();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputShowBuildWheel();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputHideBuildWheel();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBuildWheelBack();

	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBuildWheelSelect();

	// Toggle the building mechanics
	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildModeEnabled(const bool& enable);

	// Set the players team
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(const int& team);

	// Melee attack
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void MeleeAttack();

	//virtual function for ranged special attack
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void RangedAttack() { }

	//virtual function for regular special attack
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SpecialAttack() { }

	//damage handling
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent,
		class AController * EventInstigator, AActor * DamageCauser);

	//functon called to end stun
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndStun();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PunchChargeUp();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ChargePunchMove();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ChargePunchAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Stun(float StunLength);

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void ToggleDodge();

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void ToggleRush();

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void Dodge(float x, float y);
		
	// Get the build wheel
	class UBuildWheel* GetBuildWheel() const;

	// Whether or not the player is in build mode
	FORCEINLINE bool IsBuildModeEnabled() const { return this->m_bBuildingEnabled; }

	// Get the players team
	FORCEINLINE const int& GetTeam() const { return this->m_Team; }

	FORCEINLINE class UPrimaryBrush* GetPrimaryBrush() const { return this->m_PrimaryBrush; }

	FORCEINLINE void SetBuildArea(class ABuildArea *area) { this->m_BuildArea = area; }

	FORCEINLINE const bool& IsPlayerStunned() const { return this->IsStunned; }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

	//the melee collision capsule
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Attack Capsule"))
	UCapsuleComponent* MeleeCapsule;

protected:
	virtual UClass* GetDefaultClass() const { return APlayerCharacter::StaticClass(); }

	UFUNCTION()
	void ToggleBuildMode() { if(!this->m_bBlockMovement) this->SetBuildModeEnabled(!this->m_bBuildingEnabled); }

	// Follow camera 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Camera"))
	class UCameraComponent *m_Camera;

	// Camera boom positioning the camera behind the character 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Camera Boom"))
	class USpringArmComponent *m_CameraBoom;

	// Base turn rate, in deg/sec. Other scaling may affect final turn rate. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "BaseTurnRate"))
	float m_BaseTurnRate;

	// Base look up/down rate, in deg/sec. Other scaling may affect final rate. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "BaseLookUpRate"))
	float m_BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Is Attacking?"))
	bool IsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Is Stunned?"))
	bool IsStunned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", DisplayName = "Player Knockback"))
	float PlayerKnockback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stamina"))
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Stamina"))
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Health"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Max Health"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Ranged Attack Cost"))
	float RangeAttackCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Ranged Attack Damage"))
	float RangeAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Special Attack Cost"))
	float SpecialAttackCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Special Attack Damage"))
	float SpecialAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Melee Attack Damage"))
	float MeleeAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Heavy Attack Damage"))
	float HeavyAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stun Duration"))
	float StunDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Timer"))
	float m_ChargeTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level"))
	int m_ChargeLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 2 Knockback"))
	float m_ChargeL2Knockback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 3 Knockback"))
	float m_ChargeL3Knockback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 1 Damage"))
	int m_ChargeL1Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 2 Damage"))
	int m_ChargeL2Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Level 3 Damage"))
	int m_ChargeL3Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Active"))
	bool m_ChargeActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Moving"))
	bool m_ChargeMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Active"))
	bool m_ChargePunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Target"))
	FVector m_ChargeMoveTo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Speed"))
	float m_ChargeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Level 1 Distance"))
	float m_ChargeL1Dist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Level 2 Distance"))
	float m_ChargeL2Dist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Level 3 Distance"))
	float m_ChargeL3Dist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Punch", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Stun Duration"))
	float m_ChargeStun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Toggle"))
	bool m_DodgeTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Status"))
	bool m_Dodging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Distance"))
	float m_DodgeDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Charge Punch Target"))
	FVector m_DodgeTo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (AllowPrivateAccess = "true", DisplayName = "Dodge Speed"))
	float m_DodgeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Speed"))
	float m_RushSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Status"))
	bool m_Rushing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Cost"))
	float m_RushCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rush", meta = (AllowPrivateAccess = "true", DisplayName = "Rush Knockback"))
	float m_RushKnockback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Run Speed"))
	float m_RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats", meta = (AllowPrivateAccess = "true", DisplayName = "Stamina Regen"))
	float m_StamRegen;

private:
	// Result of last trace. The trace happens every tick when building mode is enabled.
	FHitResult m_TraceResult; //the result of the last trace

	// Stop the player from moving
	bool m_bBlockMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int m_Team;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Press Timer"))
	float m_PressTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Pressed"))
	bool m_bPressed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Build Enabled"))
	bool m_bBuildingEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Build Area"))
	class ABuildArea *m_BuildArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Primary Brush"))
	class UPrimaryBrush *m_PrimaryBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build", meta = (AllowPrivateAccess = "true", DisplayName = "Secondary Brush"))
	class USecondaryBrush *m_SecondaryBrush;

public:
	//Animation flags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimWalk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimKnockback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimWeakPunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimStrongPunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Flags")
	bool AnimIdle;
};
