// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "BlockData.generated.h"

UCLASS(Blueprintable)
class KING_OF_THE_CASTLE_API UBlockData : public UObject
{
	GENERATED_BODY()
	
public:
	UBlockData();

	// Get the current block count for this data
	int GetCount() const;

	// Sets the block count. This method does not take into fact the Max Count. That must be managed separately.
	void SetCount(class UPrimaryBrush *brush, const int& count);

	// Get the name id of the block this data represents
	FName GetNameId() const;

	// Get the wanted max count for the data
	FORCEINLINE const int& GetMaxCount() const { return this->m_MaxCount; }

	// Get the class type this data represents
	FORCEINLINE const TSubclassOf<class ABlock>& GetClassType() const { return this->m_ClassType; }

	// Get the material which represents this data
	FORCEINLINE UMaterialInterface* GetGhostMaterial() const { return this->m_Material; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Count", meta = (AllowPrivateAccess = "true", DisplayName = "Count"))
	int m_Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Count", meta = (AllowPrivateAccess = "true", DisplayName = "Max Count"))
	int m_MaxCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (AllowPrivateAccess = "true", DisplayName = "Ghost Material"))
	UMaterialInterface *m_Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (AllowPrivateAccess = "true", DisplayName = "Class Type"))
	TSubclassOf<class ABlock> m_ClassType;
};
