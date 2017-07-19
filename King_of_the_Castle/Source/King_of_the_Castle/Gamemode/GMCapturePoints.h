// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Gamemode/BaseGameMode.h"
#include "GMCapturePoints.generated.h"

/**
 * 
 */
UCLASS()
class KING_OF_THE_CASTLE_API AGMCapturePoints : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AGMCapturePoints();

	virtual void OnCapturePointEnter(class ACapturePoint *point, class APlayerCharacter *character);

	virtual void OnCapturePointExit(class ACapturePoint *point, class APlayerCharacter *character);

	virtual void OnCapturePointCaptureBegin(class ACapturePoint *point, const int& team);

	virtual void OnCapturePointCaptureEnd(class ACapturePoint *point, const int& team);

	virtual void OnCapturePointCaptured(class ACapturePoint *point);

	virtual void OnCapturePointReleased(class ACapturePoint *point);

	FORCEINLINE const float& GetCaptureDuration() const { return this->m_CaptureDuration; }

	FORCEINLINE const bool& IsNeutralStateEnabled() const { return this->m_bHasNeutralState; }

	FORCEINLINE const bool& IsSpeedDependantOnMembers() const { return this->m_bMemberCountSpeed; }

protected:
	FORCEINLINE int& GetCapturePointCount(const int& team)
	{
		if (!this->m_CaptureCount.Contains(team))
		{
			this->m_CaptureCount.Add(team, 0);
		}
		return this->m_CaptureCount[team];
	}

	// How long it should take to capture a capture point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (DisplayName = "Capture Duration"))
	float m_CaptureDuration;
	
	// Whether or not there should be an intermediate stage between capturing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (DisplayName = "Has Neutral State"))
	bool m_bHasNeutralState;

	// Whether or not the speed of capturing should be sped up whenever there are more team members (x2 members = x2 speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (DisplayName = "Member Count Speed"))
	bool m_bMemberCountSpeed;

private:
	TMap<int, int> m_CaptureCount;
};
