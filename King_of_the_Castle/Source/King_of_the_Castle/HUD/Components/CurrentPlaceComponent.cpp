#include "King_of_the_Castle.h"
#include "CurrentPlaceComponent.h"

#include "Gamemode/BaseGameMode.h"
#include "Character/PlayerCharacter.h"

#define PLACE_TEXT_COLOR FLinearColor(0.9f, 0.9f, 0.9f, 0.9f)

UCurrentPlaceComponent::UCurrentPlaceComponent()
{
	this->m_PlaceTextScale = 1.5f;
	this->m_PlaceSubTextScale = 0.5f;
	this->m_PlaceTextColor = PLACE_TEXT_COLOR;

	Super::m_Padding = FVector2D(10.0f, 0.0f);
}

void UCurrentPlaceComponent::Render(AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	ABaseGameMode *gamemode = GetGameMode<ABaseGameMode>(hud->GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	int32 place = gamemode->GetPlace(hud->GetCharacter()->GetTeam());
	FString placeStr = FString::FromInt(place), subStr = place == 1 ? TEXT("st")
		: place == 2 ? TEXT("nd") : place == 3 ? TEXT("rd") : TEXT("th");

	float numWidth, numHeight, subWidth, subHeight;
	hud->GetTextSize(placeStr, numWidth, numHeight, hud->GetFont(), this->m_PlaceTextScale * scale);
	hud->GetTextSize(subStr, subWidth, subHeight, hud->GetFont(), this->m_PlaceSubTextScale * scale);

	float x = (hud->IsOpposite() ? (origin.X + extent.X - numWidth) : origin.X) - subWidth / 2.0f;
	float y = origin.Y + extent.Y - numHeight;

	hud->DrawText(placeStr, this->m_PlaceTextColor, x, y, hud->GetFont(), this->m_PlaceTextScale * scale);

	hud->DrawText(subStr, this->m_PlaceTextColor, x + numWidth, y, hud->GetFont(), this->m_PlaceSubTextScale * scale);
}