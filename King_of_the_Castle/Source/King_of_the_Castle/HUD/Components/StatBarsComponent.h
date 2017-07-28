#pragma once

#include "HUD/HUDComponent.h"
#include "StatBarsComponent.generated.h"

UCLASS(Blueprintable)
class KING_OF_THE_CASTLE_API UStatBarsComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UStatBarsComponent();

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

private:
	float m_HealthPerc, m_StaminaPerc;
};
