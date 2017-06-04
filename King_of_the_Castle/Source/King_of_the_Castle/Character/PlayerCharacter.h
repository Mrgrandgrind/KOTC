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

	// Called whenever the player moved up a block type with their brush
	UFUNCTION(BlueprintCallable, Category = "Event")
	void InputBlockTypeUpEvent();

	// Called whenever the player moves down a block type with their brush`
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

	// Toggle the building mechanics
	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetBuildModeEnabled(const bool& enable);

	// Set the players team
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(const int& team);

	// Whether or not the player is in build mode
	FORCEINLINE bool IsBuildModeEnabled() const { return this->m_bBuildingEnabled; }

	// Get the players team
	FORCEINLINE const int& GetTeam() const { return this->m_Team; }

	FORCEINLINE class UPrimaryBrush* GetPrimaryBrush() const { return this->m_PrimaryBrush; }

	FORCEINLINE void SetBuildArea(class ABuildArea *area) { this->m_BuildArea = area; }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

protected:
	virtual UClass* GetDefaultClass() const { return APlayerCharacter::StaticClass(); }

	UFUNCTION()
	void ToggleBuildMode() { this->SetBuildModeEnabled(!this->m_bBuildingEnabled); }

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

private:
	// Result of last trace. The trace happens every tick when building mode is enabled.
	FHitResult m_TraceResult; //the result of the last trace

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
	
	// DEBUG VARIABLES //
	// Draw the line trace for block placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true", DisplayName = "Draw trace"))
	bool m_bDebugDrawTrace;
	/////////////////////
};
