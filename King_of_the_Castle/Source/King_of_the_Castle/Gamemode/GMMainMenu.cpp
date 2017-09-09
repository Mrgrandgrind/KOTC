// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GMMainMenu.h"

#include "SplineTrack.h"
#include "Character/DefaultPlayerCharacter.h"

#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"

#define DEFAULT_SP_TRACK_DURATION 40.0f
#define DEFAULT_CS_TRACK_DURATION 2.0f

#define MENU_CAMERA_NAME TEXT("MenuCamera")
#define MENU_ROTATION_OFFSET FRotator(-20.0f, -10.0f, 3.0f) 
#define MENU_POSITION_OFFSET FVector(1000.0f, -200.0f, -650.0f) 

#define CHARACTER_SELECT_CAMERA_NAME TEXT("CharacterSelectCamera")
#define CHARACTER_SELECT_POSITION_GAP 100.0f
#define CHARACTER_SELECT_POSITION_OFFSET FVector(0.0f, 250.0f, 0.0f)
 
AGMMainMenu::AGMMainMenu() : m_Display(EMenuDisplay::Splash)
{
	this->m_Counter = 0.0f;
	this->m_MenuActor = nullptr;
	this->m_Character = nullptr;
	this->m_SPTrack = nullptr;
	this->m_CSTrack = nullptr;

	this->m_SPTrackName = TEXT("SplashTrack");
	this->m_SPTrackDuration = DEFAULT_SP_TRACK_DURATION;

	this->m_MenuCameraName = MENU_CAMERA_NAME;
	this->m_MenuRotationOffset = MENU_ROTATION_OFFSET;
	this->m_MenuPositionOffset = MENU_POSITION_OFFSET;

	this->m_CSCameraName = CHARACTER_SELECT_CAMERA_NAME;
	this->m_CSPositionGap = CHARACTER_SELECT_POSITION_GAP;
	this->m_CSPositionOffset = CHARACTER_SELECT_POSITION_OFFSET;
	this->m_CSTrackName = TEXT("CharacterSelectTrack");
	this->m_CSTrackDuration = DEFAULT_CS_TRACK_DURATION;

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
	for(int i = 0; i < out.Num(); i++)
	{
		if(out[i]->GetName() == this->m_SPTrackName.ToString())
		{
			this->m_SPTrack = Cast<ASplineTrack>(out[0]);
			this->m_SPTrack->GetSpline()->Duration = this->m_SPTrackDuration;

			// Start at random point on track by manipulating the time
			this->m_Counter += this->m_SPTrackDuration * FMath::FRand();
		}
		if(out[i]->GetName() == this->m_CSTrackName.ToString())
		{
			this->m_CSTrack = Cast<ASplineTrack>(out[0]);
			this->m_CSTrack->GetSpline()->Duration = this->m_CSTrackDuration;
		}
	}
	if(this->m_SPTrack == nullptr)
	{
		UE_LOG(LogClass, Error, TEXT("[GMMainMenu] Unable to find splash track in map"));
	}
	if(this->m_CSTrack == nullptr)
	{
		UE_LOG(LogClass, Error, TEXT("[GMMainMenu] Unable to find camera select track in map"))
	}

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
	USplineComponent *spline = this->m_SPTrack->GetSpline();
	location = spline->GetLocationAtTime(FMath::Fmod(this->m_Counter, 
		spline->Duration), ESplineCoordinateSpace::World, false);
	rotation = (this->m_SPTrack->GetPivot() - location).Rotation();
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

		FVector loc = this->m_MenuCameraTransform.GetLocation() + this->m_MenuCameraTransform.GetRotation().RotateVector(this->m_MenuPositionOffset);
		FRotator rot = this->m_MenuCameraTransform.GetRotation().Rotator() + this->m_MenuRotationOffset + FRotator(0.0f, 180.0f, 0.0f);
		this->m_MenuActor = Super::GetWorld()->SpawnActor<AActor>(this->m_MenuActorClass, loc, rot);
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

	if (this->m_Character == nullptr || this->m_SPTrack == nullptr)
	{
		return;
	}
	this->m_Counter += delta;
	
	FVector location;
	FRotator rotation;

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

	if (name == GET_MEMBER_NAME_CHECKED(AGMMainMenu, m_SPTrackDuration) && this->m_SPTrack != nullptr)
	{
		this->m_SPTrack->GetSpline()->Duration = this->m_SPTrackDuration;
	}
}
#endif