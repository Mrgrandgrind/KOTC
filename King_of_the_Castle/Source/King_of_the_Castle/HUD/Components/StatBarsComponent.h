#pragma once

#include "HUD/HUDComponent.h"
#include "StatBarsComponent.generated.h"

UCLASS(Blueprintable)
class KING_OF_THE_CASTLE_API UStatBarsComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UStatBarsComponent();

	FORCEINLINE void FlashStamina() { this->m_bFlashStamina = true; this->m_FlashStaminaCounter = 0.0f; }

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Health Color Max"))
	FLinearColor m_HealthMaxColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Health Color Min"))
	FLinearColor m_HealthMinColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Stamina Color"))
	FLinearColor m_StaminaColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Health Text"))
	FString m_HealthText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Stamina Text"))
	FString m_StaminaText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Scale"))
	float m_TextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Scale X Health"))
	float m_ScaleXHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Scale X Stamina"))
	float m_ScaleXStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Scale Y"))
	float m_ScaleY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Y Separation"))
	float m_SeparationY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Bar Move Speed"))
	float m_MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Stamina Flash Speed"))
	float m_StaminaFlashSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Stamina Flash Duration"))
	float m_StaminaFlashDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Stamina Flash Color"))
	FLinearColor m_StaminaFlashColor;

private:
	bool m_bFlashStamina;
	float m_FlashStaminaCounter;

	float m_HealthPerc, m_StaminaPerc;
};
