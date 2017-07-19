#pragma once

#include "GameFramework/Actor.h"
#include "CapturePoint.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ACapturePoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACapturePoint();

	virtual void Tick(float delta) override;

	void SetCapturing(const bool& capturing, const int& team);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
		UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
		UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

	FORCEINLINE const int& GetOwningTeam() const { return this->m_OwningTeam; }

	FORCEINLINE const FName& GetPointName() const { return this->m_PointName; }

protected:
	void UpdateSignalLight() const;

	// Returns true if team is constant. False if there are multiple teams or no team.
	bool GetHoldingTeam(int& team) const;

	float m_CaptureCounter;

	bool m_bCapturing;

	int m_CapturingTeam;

	// Players inside this capture point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Point", meta = (DisplayName = "Name"))
	FName m_PointName;

	// Current owner of this capture point. 0 means neutral
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture Point", meta = (DisplayName = "Team Owner"))
	int m_OwningTeam;

	UPROPERTY()
	TArray<class APlayerCharacter*> m_Players;

	UPROPERTY()
	UPointLightComponent *m_SignalLight;
};
