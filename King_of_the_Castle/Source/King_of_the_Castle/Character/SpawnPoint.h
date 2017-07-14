#pragma once

#include "GameFramework/PlayerStart.h"
#include "SpawnPoint.generated.h"

UCLASS()
class KING_OF_THE_CASTLE_API ASpawnPoint : public APlayerStart
{
	GENERATED_BODY()
	
public:
	ASpawnPoint();

	FORCEINLINE const int& GetTeam() const { return this->m_Team; }

	FORCEINLINE const bool& IsTeamIgnored() const { return this->m_bIgnoreTeam; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Team"))
	int m_Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team", meta = (AllowPrivateAccess = "true", DisplayName = "Ignore Team"))
	bool m_bIgnoreTeam;
};
