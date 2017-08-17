#pragma once

#include "Event/GameEvent.h"
#include "FloorIsLavaEvent.generated.h"

#define FLOOR_IS_LAVA_NAME TEXT("Floor is Lava")

UCLASS()
class KING_OF_THE_CASTLE_API AFloorIsLavaEvent : public AGameEvent
{
	GENERATED_BODY()
	
public:
	AFloorIsLavaEvent();

	virtual void Tick(float delta) override;

	virtual void Start() override;

	virtual void Stop() override;

	virtual FName GetEventName() const override { return FLOOR_IS_LAVA_NAME; }

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials", meta = (AllowPrivateAccess = "true", DisplayName = "Lava"))
	UMaterialInterface *m_Lava;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event", meta = (AllowPrivateAccess = "true", DisplayName = "Floor"))
	class AStaticMeshActor *m_Floor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (AllowPrivateAccess = "true", DisplayName = "Trigger Component"))
	UBoxComponent *m_Trigger;

	UPROPERTY()
	UMaterialInterface *m_PreviousMaterial;

	float m_LastAlpha;

	TArray<class APlayerCharacter*> m_Overlapping;
};
