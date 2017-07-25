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

	float GetCapturePercentage() const;

	FORCEINLINE const int& GetOwningTeam() const { return this->m_OwningTeam; }

	FORCEINLINE const FName& GetPointName() const { return this->m_PointName; }

	FORCEINLINE const float& GetScoreMultiplier() const { return this->m_ScoreMultiplier; }

	FORCEINLINE const float& GetScorePerCapture() const { return this->m_ScorePerCapture; }

	FORCEINLINE const bool& IsBeingCaptured() const { return this->m_bCapturing; }

	FORCEINLINE const int& GetCapturingTeam() const { return this->m_CapturingTeam; }

protected:
	// Set light to represet owning team
	void UpdateSignalLight();

	// Returns true if team is constant. False if there are multiple teams or no team.
	bool GetHoldingTeam(int& team) const;

	bool m_bCapturing;

	int m_CapturingTeam;

	float m_CaptureCounter;

	// Players inside this capture point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Point", meta = (DisplayName = "Name"))
	FName m_PointName;

	// Score multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Point", meta = (DisplayName = "Score Multiplier"))
	float m_ScoreMultiplier;

	// Score multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture Point", meta = (DisplayName = "Score Per Capture"))
	float m_ScorePerCapture;

	// Current owner of this capture point. 0 means neutral
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture Point", meta = (DisplayName = "[Debug] Team Owner"))
	int m_OwningTeam;

	// All players in this capture point
	UPROPERTY()
	TArray<class APlayerCharacter*> m_Players;

	// Owning team signal light
	UPROPERTY()
	UPointLightComponent *m_SignalLight;
};
