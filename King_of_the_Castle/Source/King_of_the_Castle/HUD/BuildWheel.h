// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "BuildWheel.generated.h"

struct KING_OF_THE_CASTLE_API WheelMenuSegment
{
	FString m_Name;
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

	void Render(class AHUD *display, FVector2D& screen) const;

	FORCEINLINE const bool& IsVisible() const { return this->m_bIsVisible; }

	FORCEINLINE void SetVisible(const bool& visible) { this->m_bIsVisible = visible; }
	
protected:
	bool m_bIsVisible;

	FLinearColor m_PrimaryColor, m_SecondaryColor;
	
private:
	WheelMenu m_RootMenu;
	WheelMenu *m_CurrentMenu;

	UPROPERTY()
	UMaterialInterface *m_WheelMaterial;

	UPROPERTY()
	UMaterialInterface *m_InnerWheelMaterial;
};
