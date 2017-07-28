#pragma once

#include "HUD/HUDComponent.h"
#include "CurrentPlaceComponent.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API UCurrentPlaceComponent : public UHUDComponent
{
	GENERATED_BODY()
	
public:
	UCurrentPlaceComponent();

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Place Text Scale"))
	float m_PlaceTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Sub Text Scale"))
	float m_PlaceSubTextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Text Color"))
	FLinearColor m_PlaceTextColor;
};
