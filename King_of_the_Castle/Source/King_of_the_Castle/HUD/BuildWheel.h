// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "BuildWheel.generated.h"

struct KING_OF_THE_CASTLE_API WheelMenuSegment
{
	FString name;
};

struct KING_OF_THE_CASTLE_API WheelSelectData
{
	bool updated = false;

	int index = -1;
	float theta_begin, theta_end;
};

class KING_OF_THE_CASTLE_API WheelMenu
{
	friend class UBuildWheel;

public:
	WheelMenu(FString name = TEXT("menu"), WheelMenu *parent = nullptr) : m_Name(name), m_Parent(parent) { }

private:
	FString m_Name;

	WheelMenu *m_Parent;

	TArray<WheelMenuSegment> m_Segments;
};

UCLASS()
class KING_OF_THE_CASTLE_API UBuildWheel : public UObject
{
	GENERATED_BODY()
	
public:
	UBuildWheel();

	void SetTeam(const int& team);

	void Render(class AHUD *display, FVector2D& screen);

	void SetSelected(float radians);

	FORCEINLINE void SetSelected(const int& index) { this->m_SelectData.index = index; this->m_SelectData.updated = false; }

	FORCEINLINE const bool& IsVisible() const { return this->m_bIsVisible; }

	FORCEINLINE void SetVisible(const bool& visible) { this->m_bIsVisible = visible; }

	FORCEINLINE bool IsMenuValid() const { return this->m_CurrentMenu != nullptr && this->m_CurrentMenu->m_Segments.Num() != 0;  }
	
protected:
	void UpdateSelectMaterial() const;

	bool m_bIsVisible;

	FLinearColor m_PrimaryColor, m_SecondaryColor;
	
private:
	WheelMenu m_RootMenu;
	WheelMenu *m_CurrentMenu;

	WheelSelectData m_SelectData;

	UPROPERTY()
	UMaterialInterface *m_WheelMaterial;

	UPROPERTY()
	UMaterialInterface *m_InnerWheelMaterial;

	UPROPERTY()
	UMaterialInterface *m_SelectMaterial;
};
