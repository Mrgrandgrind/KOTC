// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <stack>
#include <functional>

#include "UObject/NoExportTypes.h"
#include "BuildWheel.generated.h"

struct KING_OF_THE_CASTLE_API WheelMenuSegment
{
	FString name;
	std::function<bool(class APlayerCharacter *character)> action; //return true keeps menu open and false closes it
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
	WheelMenu(FString name = TEXT("unnamed")) : m_Name(name) { }

private:
	FORCEINLINE void AddSegment(const WheelMenuSegment& segment) { this->m_Segments.push_back(segment); }

	FString m_Name;

	//TArray<WheelMenuSegment> m_Segments;
	std::vector<WheelMenuSegment> m_Segments;
};

UCLASS()
class KING_OF_THE_CASTLE_API UBuildWheel : public UObject
{
	GENERATED_BODY()
	
public:
	UBuildWheel();

	void SetTeam(const int& team);

	void Render(class AHUD *display, FVector4& screen);

	void SetSelected(float radians);

	void Select(class APlayerCharacter *character);

	void Back();

	FORCEINLINE void SetSelected(const int& index) { this->m_SelectData.index = index; this->m_SelectData.updated = false; }

	FORCEINLINE const bool& IsVisible() const { return this->m_bIsVisible; }

	FORCEINLINE void SetVisible(const bool& visible) { this->m_bIsVisible = visible; while (this->m_MenuStack.size() > 1) this->m_MenuStack.pop(); }

	FORCEINLINE WheelMenu& GetActiveMenu()
	{
		if(this->m_MenuStack.size() == 0)
		{
			// If there is no active menu, set it to the default.
			this->OpenMenu(this->CreateWheelMenu());
			check(this->m_MenuStack.size() != 0);
		}
		return this->m_MenuStack.top();
	}
	
protected:
	WheelMenu CreateWheelMenu();

	void UpdateSelectMaterial() const;

	FORCEINLINE bool OpenMenu(WheelMenu menu)
	{
		this->m_MenuStack.push(menu);
		this->m_SelectData.index = -1;
		this->m_SelectData.updated = false;
		return true;
	}

	bool m_bIsVisible;

	FLinearColor m_PrimaryColor, m_SecondaryColor;
	
private:
	std::stack<WheelMenu> m_MenuStack;

	WheelSelectData m_SelectData;

	UPROPERTY()
	UMaterialInterface *m_WheelMaterial;

	UPROPERTY()
	UMaterialInterface *m_InnerWheelMaterial;

	UPROPERTY()
	UMaterialInterface *m_SelectMaterial;
};
