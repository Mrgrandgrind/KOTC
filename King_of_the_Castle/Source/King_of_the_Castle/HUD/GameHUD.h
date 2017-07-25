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

	//FORCEINLINE UBuildWheel* GetBuildWheel() const { return this->m_BuildWheel; }

	FORCEINLINE void SetCrosshairVisible(const bool& visible) { this->m_bCrosshairVisible = visible; }

	FORCEINLINE bool IsViewportVertical() const { return this->m_PlayerCount <= 2; }

private:
	FORCEINLINE class APlayerCharacter* GetCharacter() const { return (APlayerCharacter*)Super::GetOwningPawn(); }

	void RenderBars(const FVector4& screen, const float& scale);

	void RenderCapturePoints(const FVector4& screen, const float& scale);

	void RenderForAll(const FVector4& screen, const float& scale);

protected:
	//UPROPERTY(VisibleAnywhere)
	//UBuildWheel *m_BuildWheel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Font"))
	class UFont *m_Font;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Box Owned Color"))
	FLinearColor m_CPOwnedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Box Unowned Color"))
	FLinearColor m_CPUnownedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Box Scale"))
	float m_CPBoxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Team Box UV Size"))
	float m_CPTeamBoxUVSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Box Alpha"))
	float m_CPBoxAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Box Flash Speed"))
	float m_CPBoxFlashSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Text Color"))
	FLinearColor m_CPTextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Text Scale"))
	float m_CPTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Text Y Offset"))
	float m_CPTextZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Capture Point Material"))
	UMaterialInterface *m_CPMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Capture Points", meta = (DisplayName = "Render Capture Points"))
	bool m_bRenderCapturePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Time Box Scale"))
	float m_IPTimeBoxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Time Text Scale"))
	float m_IPTimeTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Time Text Color"))
	FLinearColor m_IPTimeTextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Render Time"))
	bool m_bRenderTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Team Bar Width"))
	float m_IPTeamBarWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Team Bar Min Height"))
	float m_IPTeamBarMinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Team Bar Max Height"))
	float m_IPTeamBarMaxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Team Bar Alpha"))
	float m_IPTeamBarAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Info Panel", meta = (DisplayName = "Render Info Panel"))
	bool m_bRenderInfoPanel;

private:
	int32 m_ControllerId, m_PlayerCount;

	bool m_bCrosshairVisible;

	// Each flag will have their own material instance because DrawMaterial doesn't immediately draw and material parameters get overwritten
	UPROPERTY()
	TMap<class ACapturePoint*, UMaterialInterface*> m_CPMaterials;
};
