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

	FORCEINLINE bool IsViewportVertical() const { return this->m_PlayerCount <= 2; }

	FORCEINLINE class UFont* GetFont() const { return this->m_Font; }

	FORCEINLINE const int& GetPlayerCount() const { return this->m_PlayerCount; }

	FORCEINLINE const int& GetControllerId() const { return this->m_ControllerId; }

	FORCEINLINE class APlayerCharacter* GetCharacter() const { return (APlayerCharacter*)Super::GetOwningPawn(); }

	FORCEINLINE bool IsOpposite()
	{
		return (this->m_PlayerCount == 2 && this->m_ControllerId == 1)
			|| (this->m_PlayerCount == 3 && this->m_ControllerId == 2)
			|| (this->m_PlayerCount == 4 && (this->m_ControllerId == 1 || this->m_ControllerId == 3));
	}

	template<typename T>
	FORCEINLINE T* FindComponent()
	{
		for (UObject *next : this->m_Components)
		{
			T* obj = Cast<T>(next);
			if (obj != nullptr)
			{
				return obj;
			}
		}
		return nullptr;
	}

	template<typename T>
	FORCEINLINE static T* FindComponent(APlayerController *controller)
	{
		AGameHUD *hud = Cast<AGameHUD>(controller->GetHUD());
		if (hud == nullptr)
		{
			return nullptr;
		}
		return hud->FindComponent<T>();
	}

	template<typename T>
	FORCEINLINE static T* FindComponent(APlayerCharacter *character)
	{
		return AGameHUD::FindComponent<T>((APlayerController*)character->GetController());
	}

private:
	void RenderScores(const FVector4& screen, const float& scale);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Font"))
	class UFont *m_Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Scale", meta = (DisplayName = "Master Scale"))
	float m_ScaleMaster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Scale", meta = (DisplayName = "Vertical Viewport Scale (1/2 Player)"))
	float m_ScaleVertical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD|Scale", meta = (DisplayName = "Horizontal Viewport Scale (3/4 Player)"))
	float m_ScaleHorizontal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameHUD", meta = (DisplayName = "Elements"))
	TArray<TSubclassOf<class UHUDComponent>> m_ComponentClasses;

private:
	int32 m_ControllerId, m_PlayerCount;

	UPROPERTY()
	TArray<class UHUDComponent*> m_Components;
};
