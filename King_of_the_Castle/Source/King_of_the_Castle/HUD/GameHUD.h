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

	FORCEINLINE UBuildWheel* GetBuildWheel() const { return this->m_BuildWheel; }

	FORCEINLINE void SetCrosshairVisible(const bool& visible) { this->m_bCrosshairVisible = visible; }

	FORCEINLINE bool IsViewportVertical() const { return this->m_PlayerCount <= 2; }

private:
	FORCEINLINE class APlayerCharacter* GetCharacter() const { return (APlayerCharacter*)Super::GetOwningPawn(); }

	void RenderBars(const FVector4& screen, const float& scale);

protected:
	UPROPERTY()
	UBuildWheel *m_BuildWheel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Scale", meta = (DisplayName = "Master Scale"))
	float m_ScaleMaster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Scale", meta = (DisplayName = "Vertical Viewport Scale (1/2 Player)"))
	float m_ScaleVertical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Scale", meta = (DisplayName = "Horizontal Viewport Scale (3/4 Player)"))
	float m_ScaleHorizontal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Health Color"))
	FLinearColor m_BarHealthColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Stamina Color"))
	FLinearColor m_BarStaminaColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Health Text"))
	FString m_BarHealthText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Stamina Text"))
	FString m_BarStaminaText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Text Scale"))
	float m_BarTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Bar Scale X Health"))
	float m_BarScaleXHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Bar Scale X Stamina"))
	float m_BarScaleXStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Bar Scale Y"))
	float m_BarScaleY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Bar Separation"))
	float m_BarSeparation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Bar Padding"))
	float m_BarPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Health & Stamina Bars", meta = (DisplayName = "Render Bars"))
	bool m_bRenderBars;

private:
	int32 m_ControllerId, m_PlayerCount;

	bool m_bCrosshairVisible;
};
