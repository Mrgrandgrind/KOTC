// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMCapturePoints.h"

#include "Character/PlayerCharacter.h"
#include "Gamemode/GMCapturePoints/CapturePoint.h"

#include "Runtime/Engine/Classes/Engine/Engine.h"

#define DEFAULT_CAPTURE_DURATION 6.0f

AGMCapturePoints::AGMCapturePoints() : m_CaptureDuration(DEFAULT_CAPTURE_DURATION), m_bHasNeutralState(false), m_bMemberCountSpeed(true)
{
}

void AGMCapturePoints::OnCapturePointEnter(ACapturePoint *point, APlayerCharacter *character)
{
}

void AGMCapturePoints::OnCapturePointExit(ACapturePoint *point, APlayerCharacter *character)
{
}

void AGMCapturePoints::OnCapturePointCaptureBegin(class ACapturePoint *point, const int& team)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
		TEXT("%s is being captured by team %d!"), *point->GetPointName().ToString(), team));
}

void AGMCapturePoints::OnCapturePointCaptureEnd(class ACapturePoint *point, const int& team)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
		TEXT("%s is no longer being captured by team %d!"), *point->GetPointName().ToString(), team));
}

void AGMCapturePoints::OnCapturePointCaptured(ACapturePoint *point)
{
	int count = ++this->GetCapturePointCount(point->GetOwningTeam());

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
		TEXT("%s was captured by team %d! They now control %d points!"), *point->GetPointName().ToString(), point->GetOwningTeam(), count));
}

void AGMCapturePoints::OnCapturePointReleased(ACapturePoint *point)
{
	int count = --this->GetCapturePointCount(point->GetOwningTeam());

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(
		TEXT("%s is no longer owned by team %d! They now control %d points!"), *point->GetPointName().ToString(), point->GetOwningTeam(), count));
}