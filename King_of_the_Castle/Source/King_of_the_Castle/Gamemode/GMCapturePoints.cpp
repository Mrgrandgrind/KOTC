// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMCapturePoints.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#include "Runtime/Engine/Classes/Engine/Engine.h"

#define DEFAULT_CAPTURE_DURATION 6.0f

AGMCapturePoints::AGMCapturePoints() : m_CaptureDuration(DEFAULT_CAPTURE_DURATION), 
m_ScorePerSecond(0.25f), m_bHasNeutralState(false), m_bMemberCountSpeed(true), m_WinScore(10.0f)
{
}

void AGMCapturePoints::Tick(float delta)
{
	Super::Tick(delta);

	if (Super::IsGameOver())
	{
		return;
	}

	TMap<int, float>& score = Super::GetScores();
	for (TPair<int, int>& pair : this->m_CaptureCount)
	{
		if (!score.Contains(pair.Key))
		{
			score.Add(pair.Key);
		}
		score[pair.Key] += delta * this->m_ScorePerSecond * pair.Value * this->m_Multiplier[pair.Key];

		//UE_LOG(LogClass, Log, TEXT("Team %d: %f"), pair.Key, score[pair.Key]);

		if (score[pair.Key] >= this->m_WinScore)
		{
			Super::EndGame(pair.Key);
			return;
		}
	}
}

void AGMCapturePoints::OnCapturePointEnter_Implementation(ACapturePoint *point, APlayerCharacter *character)
{
}

void AGMCapturePoints::OnCapturePointExit_Implementation(ACapturePoint *point, APlayerCharacter *character)
{
}

void AGMCapturePoints::OnCapturePointCaptureBegin_Implementation(class ACapturePoint *point, const int& team)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
	//	TEXT("%s is being captured by team %d!"), *point->GetPointName().ToString(), team));
}

void AGMCapturePoints::OnCapturePointCaptureEnd_Implementation(class ACapturePoint *point, const int& team)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
	//	TEXT("%s is no longer being captured by team %d!"), *point->GetPointName().ToString(), team));
}

void AGMCapturePoints::OnCapturePointCaptured_Implementation(ACapturePoint *point)
{
	int count = ++this->GetCaptureCount(point->GetOwningTeam());

	TMap<int, float>& score = Super::GetScores();
	if (!score.Contains(point->GetOwningTeam()))
	{
		score.Add(point->GetOwningTeam(), 0.0f);
	}
	score[point->GetOwningTeam()] += point->GetScorePerCapture();
	this->m_Multiplier[point->GetOwningTeam()] += point->GetScoreMultiplier();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
	//	TEXT("%s was captured by team %d! They now control %d points!"), *point->GetPointName().ToString(), point->GetOwningTeam(), count));
}

void AGMCapturePoints::OnCapturePointReleased_Implementation(ACapturePoint *point)
{
	int count = --this->GetCaptureCount(point->GetOwningTeam());
	this->m_Multiplier[point->GetOwningTeam()] -= point->GetScoreMultiplier();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
	//	TEXT("%s is no longer owned by team %d! They now control %d points!"), *point->GetPointName().ToString(), point->GetOwningTeam(), count));
}