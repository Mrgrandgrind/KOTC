#pragma once

#include "HUD/HUDComponent.h"
#include "CountdownComponent.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API UCountdownComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UCountdownComponent();

	bool IsCountdownComplete() { return !Super::m_bRender || this->m_Counter > this->m_Duration + this->m_WaitDuration; }

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Duration"))
	float m_Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Go Duration"))
	float m_GoDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Wait Duration"))
	float m_WaitDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Scale"))
	float m_TextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Color"))
	FLinearColor m_TextColor;

private:
	float m_Counter;
};
