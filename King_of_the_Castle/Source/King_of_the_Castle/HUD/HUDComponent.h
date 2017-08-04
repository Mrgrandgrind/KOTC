// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HUD/GameHUD.h"
#include "UObject/NoExportTypes.h"
#include "HUDComponent.generated.h"

UCLASS(Blueprintable)
class KING_OF_THE_CASTLE_API UHUDComponent : public UObject
{
	GENERATED_BODY()

	friend class AGameHUD;

public:
	UHUDComponent();

	void DrawComponent(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale, const bool& forceRender = false);

	FORCEINLINE virtual void SetVisible(const bool& visible) { this->m_bRender = visible; }

	FORCEINLINE const bool& IsVisible() const { return this->m_bRender; }

protected:
	virtual void Render(class AGameHUD *hud, const FVector2D& origin, const FVector2D& extent, const float& scale) { }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (DisplayName = "Scale"))
	float m_MasterScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (DisplayName = "Padding"))
	FVector2D m_Padding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (DisplayName = "Origin Offset"))
	FVector2D m_OriginOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (DisplayName = "Render"))
	bool m_bRender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (DisplayName = "Render On Last"))
	bool m_bRenderOnLast;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component", meta = (DisplayName = "Render when Paused"))
	bool m_bRenderWhenPaused;
};