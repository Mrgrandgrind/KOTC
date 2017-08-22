// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMMainMenu.h"

#include "SplineTrack.h"
#include "Character/DefaultPlayerCharacter.h"

#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"

#define DEFAULT_TRACK_DURATION 40.0f
#define CAMERA_PIVOT_POINT FVector(13.0f, -3389.0f, 700.0f)

#define MENU_CAMERA_NAME TEXT("MenuCamera")
#define MENU_ROTATION FRotator(0.0f, 35.0f, 0.0f)
#define MENU_POSITION_OFFSET FVector(-1000.0f, -700.0f, -275.0f)

#define CHARACTER_SELECT_CAMERA_NAME TEXT("CharacterSelectCamera")
#define CHARACTER_SELECT_POSITION_GAP 100.0f
#define CHARACTER_SELECT_POSITION_OFFSET FVector(0.0f, 250.0f, 0.0f)

AGMMainMenu::AGMMainMenu() : m_Duration(DEFAULT_TRACK_DURATION), /*m_Pivot(CAMERA_PIVOT_POINT),*/ m_Display(EMenuDisplay::Splash)
{
	this->m_MenuCameraName = MENU_CAMERA_NAME;
	this->m_MenuRotation = MENU_ROTATION;
	this->m_MenuPositionOffset = MENU_POSITION_OFFSET;

	this->m_CSCameraName = CHARACTER_SELECT_CAMERA_NAME;
	this->m_CSPositionGap = CHARACTER_SELECT_POSITION_GAP;
	this->m_CSPositionOffset = CHARACTER_SELECT_POSITION_OFFSET;

	Super::DefaultPawnClass = ADefaultPlayerCharacter::StaticClass();
}

void AGMMainMenu::BeginPlay()
{
	Super::BeginPlay();

	if (Super::GetWorld() == nullptr)
	{
		return;
	}
	TArray<AActor*> out;

	// Locate the camera track
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ASplineTrack::StaticClass(), out);
	if (out.Num() == 0)
	{
		UE_LOG(LogClass, Error, TEXT("[GMMainMenu] No ASplineTrack in map!"));
	}
	else
	{
		if (out.Num() > 1)
		{
			UE_LOG(LogClass, Error, TEXT("[GMMainMenu] Too many ASplineTrack's in map! Expected 1, found %d"), out.Num());
		}
		this->m_Track = Cast<ASplineTrack>(out[0]);
		this->m_Track->GetSpline()->Duration = this->m_Duration;
	}

	// Start at random point on track by manipulating the time
	this->m_Counter += this->m_Duration * FMath::FRand();

	// Locate camera
	UGameplayStatics::GetAllActorsOfClass(Super::GetWorld(), ACameraActor::StaticClass(), out);
	for (AActor *next : out)
	{
		if (!next->IsA(ACameraActor::StaticClass()))
		{
			continue;
		}
		if (next->GetName() == this->m_CSCameraName.ToString())
		{
			this->m_CSCameraTransform = next->GetTransform();
		}
		if (next->GetName() == this->m_MenuCameraName.ToString())
		{
			this->m_MenuCameraTransform = next->GetTransform();
		}
	}

	// Disable split screen whilst in main menu
	Super::GetWorld()->GetGameViewport()->SetDisableSplitscreenOverride(true);
}

APawn* AGMMainMenu::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot)
{
	APawn *pawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	if (pawn != nullptr)
	{
		this->m_Character = Cast<ADefaultPlayerCharacter>(pawn);
	}
	return pawn;
}

void AGMMainMenu::GetSplashTransform(FVector& location, FRotator& rotation) const
{
	USplineComponent *spline = this->m_Track->GetSpline();
	location = spline->GetLocationAtTime(FMath::Fmod(this->m_Counter, 
		spline->Duration), ESplineCoordinateSpace::World, false);
	rotation = (this->m_Track->GetPivot() - location).Rotation();
}

void AGMMainMenu::GetMenuTransform(FVector& location, FRotator& rotation) const
{
	location = this->m_MenuCameraTransform.GetLocation();
	rotation = this->m_MenuCameraTransform.Rotator();
}

void AGMMainMenu::GetCharacterSelectTransform(FVector& location, FRotator& rotation) const
{
	location = this->m_CSCameraTransform.GetLocation();
	rotation = this->m_CSCameraTransform.Rotator();
}

void AGMMainMenu::SetDisplay(EMenuDisplay display)
{
	if (this->m_Display == display)
	{
		return;
	}
	this->m_Display = display;
	if (display == EMenuDisplay::Menu && this->m_MenuActorClass != nullptr)
	{
		check(this->m_MenuActor == nullptr);
		this->m_MenuActor = Super::GetWorld()->SpawnActor<AActor>(this->m_MenuActorClass, 
			this->m_MenuCameraTransform.GetLocation() + this->m_MenuPositionOffset, this->m_MenuRotation);
	}
	else if (this->m_MenuActor != nullptr)
	{
		this->m_MenuActor->Destroy();
		this->m_MenuActor = nullptr;
	}
}

void AGMMainMenu::Tick(float delta)
{
	Super::Tick(delta);

	if (this->m_Character == nullptr || this->m_Track == nullptr)
	{
		return;
	}
	this->m_Counter += delta;
	
	FVector location;
	FRotator rotation;

	USplineComponent *spline = this->m_Track->GetSpline();
	if (this->m_Display == EMenuDisplay::Splash)
	{
		this->GetSplashTransform(location, rotation);
	}
	else if (this->m_Display == EMenuDisplay::Menu)
	{
		this->GetMenuTransform(location, rotation);
	}
	else if (this->m_Display == EMenuDisplay::Character)
	{
		this->GetCharacterSelectTransform(location, rotation);
	}

	this->m_Character->SetActorLocation(location);
	this->m_Character->GetCamera()->SetWorldRotation(rotation);
}

#if WITH_EDITOR
void AGMMainMenu::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(AGMMainMenu, m_Duration) && this->m_Track != nullptr)
	{
		this->m_Track->GetSpline()->Duration = this->m_Duration;
	}
}
#endif