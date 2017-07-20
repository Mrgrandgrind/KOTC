#include "King_of_the_Castle.h"
#include "CapturePoint.h"

#include "Gamemode/GMCapturePoints.h"
#include "Character/PlayerCharacter.h"

#define TEAM_NEUTRAL 0

#define TEAM1_COLOR FLinearColor(0.0f, 0.0f, 1.0f)
#define TEAM2_COLOR FLinearColor(1.0f, 0.0f, 0.0f)
#define TEAM3_COLOR FLinearColor(0.0f, 1.0f, 0.0f)
#define TEAM4_COLOR FLinearColor(1.0f, 0.0f, 1.0f)
#define TEAM_NEUTRAL_COLOR FLinearColor(1.0f, 1.0f, 1.0f)

ACapturePoint::ACapturePoint() : m_PointName(TEXT("Capture Point")), m_OwningTeam(TEAM_NEUTRAL)
{
	UBoxComponent *box = UObject::CreateDefaultSubobject<UBoxComponent>(TEXT("CaptureArea"));
	Super::RootComponent = box;

	this->m_SignalLight = UObject::CreateDefaultSubobject<UPointLightComponent>(TEXT("SignalLight"));
	this->m_SignalLight->SetIntensity(10000.0f);
	this->m_SignalLight->SetupAttachment(Super::RootComponent);

	TScriptDelegate<FWeakObjectPtr> sdb;
	sdb.BindUFunction(this, FName("OnBeginOverlap"));
	box->OnComponentBeginOverlap.Add(sdb);

	TScriptDelegate<FWeakObjectPtr> sde;
	sde.BindUFunction(this, FName("OnEndOverlap"));
	box->OnComponentEndOverlap.Add(sde);

	Super::PrimaryActorTick.bCanEverTick = true;
}

void ACapturePoint::SetCapturing(const bool& capturing, const int& team)
{
	if (this->m_bCapturing == capturing)
	{
		return;
	}
	if (capturing && team == this->m_OwningTeam)
	{
		this->m_CapturingTeam = -1;
		this->m_CaptureCounter = 0.0f;
		return;
	}
	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
	if (gamemode == nullptr)
	{
		return;
	}
	this->m_bCapturing = capturing;
	if (capturing)
	{
		gamemode->OnCapturePointCaptureBegin(this, team);

		if (this->m_CapturingTeam != team)
		{
			this->m_CaptureCounter = 0.0f;
		}
		this->m_CapturingTeam = team;
	}
	else
	{
		gamemode->OnCapturePointCaptureEnd(this, this->m_CapturingTeam);
	}
}

bool ACapturePoint::GetHoldingTeam(int& team) const
{
	if (this->m_Players.Num() == 0)
	{
		return false;
	}
	team = this->m_Players[0]->GetTeam();
	for (int i = 1; i < this->m_Players.Num(); i++)
	{
		if (this->m_Players[i]->GetTeam() != team)
		{
			return false;
		}
	}
	return true;
}

void ACapturePoint::UpdateSignalLight() const
{
	if (this->m_SignalLight == nullptr)
	{
		return;
	}
	switch (this->m_OwningTeam)
	{
		case 1:
			this->m_SignalLight->SetLightColor(TEAM1_COLOR);
			break;
		case 2:
			this->m_SignalLight->SetLightColor(TEAM2_COLOR);
			break;
		case 3:
			this->m_SignalLight->SetLightColor(TEAM3_COLOR);
			break;
		case 4:
			this->m_SignalLight->SetLightColor(TEAM4_COLOR);
			break;
		default:
			this->m_SignalLight->SetLightColor(TEAM_NEUTRAL_COLOR);
	}
}

void ACapturePoint::Tick(float delta)
{
	Super::Tick(delta);

	// Do nothing if we are not currently capturing
	if (!this->m_bCapturing)
	{
		return;
	}

	AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
	// Do nothing if we are not in a gamemode that supports capture points
	if (gamemode == nullptr)
	{
		return;
	}
	this->m_CaptureCounter += delta * (gamemode->IsSpeedDependantOnMembers() ? this->m_Players.Num() : 1.0f);

	if (this->m_CaptureCounter >= gamemode->GetCaptureDuration())
	{
		if (this->m_OwningTeam != TEAM_NEUTRAL)
		{
			gamemode->OnCapturePointReleased(this);
		}

		if (gamemode->IsNeutralStateEnabled() && this->m_OwningTeam != TEAM_NEUTRAL)
		{
			this->m_OwningTeam = TEAM_NEUTRAL;
		}
		else
		{
			this->m_bCapturing = false;
			this->m_OwningTeam = this->m_CapturingTeam;

			gamemode->OnCapturePointCaptured(this);
		}

		this->m_CaptureCounter = 0.0f;
		this->UpdateSignalLight();
	}
}

void ACapturePoint::OnBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
	UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp == nullptr || !OtherComp->IsA(UStaticMeshComponent::StaticClass()))
	{
		return;
	}
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	if (character != nullptr && !this->m_Players.Contains(character))
	{
		this->m_Players.Add(character);

		int team;
		this->SetCapturing(this->GetHoldingTeam(team), team);

		AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
		if (gamemode != nullptr)
		{
			gamemode->OnCapturePointEnter(this, character);
		}
	}
}

void ACapturePoint::OnEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
	UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp == nullptr || !OtherComp->IsA(UStaticMeshComponent::StaticClass()))
	{
		return;
	}
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	if (character != nullptr && this->m_Players.Contains(character))
	{
		this->m_Players.Remove(character);

		int team;
		this->SetCapturing(this->GetHoldingTeam(team), team);

		AGMCapturePoints *gamemode = GetGameMode<AGMCapturePoints>(Super::GetWorld());
		if (gamemode != nullptr)
		{
			gamemode->OnCapturePointExit(this, character);
		}
	}
}

