#pragma once

#include "GameFramework/GameMode.h"
#include "GMMainMenu.generated.h"

UENUM(Blueprintable)
enum class EMenuDisplay : uint8
{
	Splash,
	Menu,
	Character
};

UCLASS()
class KING_OF_THE_CASTLE_API AGMMainMenu : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGMMainMenu();

	virtual void BeginPlay() override;

	virtual void Tick(float delta) override;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot) override;

	UFUNCTION(BlueprintCallable, Category = "Display")
	virtual void SetDisplay(EMenuDisplay display);

	UFUNCTION(BlueprintPure, Category = "Display")
	virtual const EMenuDisplay& GetDisplay() const { return this->m_Display; }

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& event) override;
#endif

protected:
	void GetSplashTransform(FVector& location, FRotator& rotation) const;

	void GetMenuTransform(FVector& location, FRotator& rotation) const;

	void GetCharacterSelectTransform(FVector& location, FRotator& rotation) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash", meta = (DisplayName = "Track Name"))
	FName m_SPTrackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash", meta = (DisplayName = "Track Duration"))
	float m_SPTrackDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu", meta = (DisplayName = "Camera Name"))
	FName m_MenuCameraName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu", meta = (DisplayName = "Menu Rotation Offset"))
	FRotator m_MenuRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu", meta = (DisplayName = "Menu Position Offset"))
	FVector m_MenuPositionOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu", meta = (DisplayName = "Menu Actor Class"))
	TSubclassOf<AActor> m_MenuActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Select", meta = (DisplayName = "Camera Name"))
	FName m_CSCameraName;

	UPROPERTY(EditAnywhere, BlueprintReaDWrite, Category = "Character Select", meta = (DisplayName = "Character Position Gap"))
	float m_CSPositionGap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Select", meta = (DisplayName = "Character Position Offset"))
	FVector m_CSPositionOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Select", meta = (DisplayName = "Track Name"))
	FName m_CSTrackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Select", meta = (DisplayName = "Track Duration"))
	float m_CSTrackDuration;

private:
	float m_Counter;

	EMenuDisplay m_Display;

	UPROPERTY()
	AActor *m_MenuActor;

	UPROPERTY()
	class ASplineTrack *m_SPTrack;

	UPROPERTY()
	class ASplineTrack *m_CSTrack;

	UPROPERTY()
	class ADefaultPlayerCharacter *m_Character;

	FTransform m_CSCameraTransform, m_MenuCameraTransform;
};
