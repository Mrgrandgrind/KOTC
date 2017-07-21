// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BuildWheel.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API AGameHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AGameHUD();

	virtual void BeginPlay() override;

	virtual void DrawHUD() override;

	void SetGameOver(FString message);

	UFUNCTION(BlueprintPure, Category = "Widget")
	class UUserWidget* GetWidgetHUD() const { return this->m_WidgetHUD; }

	FORCEINLINE UBuildWheel* GetBuildWheel() const { return this->m_BuildWheel; }

	FORCEINLINE void SetCrosshairVisible(const bool& visible) { this->m_bCrosshairVisible = visible; }

protected:
	UPROPERTY()
	UBuildWheel *m_BuildWheel;

private:
	UPROPERTY()
	UFont *m_Font;

	UPROPERTY()
	UClass *m_WidgetHUDClass;

	UPROPERTY()
	class UUserWidget *m_WidgetHUD;

	bool m_bGameOver;

	FString m_Message;

	bool m_bCrosshairVisible;
};
