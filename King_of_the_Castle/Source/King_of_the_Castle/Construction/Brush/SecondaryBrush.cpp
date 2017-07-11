// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "SecondaryBrush.h"

#include "PrimaryBrush.h"
#include "../Block.h"
#include "../BlockData.h"
#include "../BuildArea.h"
#include "../../Character/PlayerCharacter.h"

#include "Runtime/Engine/Classes/Engine/TextRenderActor.h"
#include "Construction/Blocks/FlagBlock.h"

#define COMBINE_DEAD_TIME 0.4f // How many seconds before the combining begins

#define BRUSH_DESTROY_COLOR FLinearColor(0.9f, 0.0f, 0.0f, 1.0f)
#define BRUSH_COMBINE_COLOR FLinearColor(1.0f, 0.5f, 0.0f, 1.0f)

#define COMBINE_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_CombineBox.M_CombineBox'")
#define CUBE_MESH_LOCATION TEXT("StaticMesh'/Game/Meshes/M_Cube.M_Cube'")

USecondaryBrush::USecondaryBrush() : m_Mode(EModifyMode::Undefined), m_bCombining(false),
m_CombineLock(nullptr), m_CraftTimer(nullptr), m_ActiveRecipe(nullptr)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(CUBE_MESH_LOCATION);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(COMBINE_MATERIAL_LOCATION);

	this->m_CombineBox = UObject::CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CombineBox"));
	if (Mesh.Succeeded())
	{
		this->m_CombineBox->SetStaticMesh(Mesh.Object);
	}
	this->m_CombineBox->bGenerateOverlapEvents = false;
	this->m_CombineBox->SetCollisionProfileName(TEXT("NoCollision"));
	this->m_CombineBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	this->m_CombineBox->SetMaterial(0, UMaterialInstanceDynamic::Create(Material.Object, Super::GetOuter()));
	this->m_CombineBox->SetRelativeLocation(FVector(0.0f, -Super::Bounds.BoxExtent.Y, -Super::Bounds.BoxExtent.Z));
	this->m_CombineBox->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	this->m_CombineBox->SetWorldScale3D(FVector(BLOCK_DEFAULT_SCALE / 100.0f + 0.75f));
	this->m_CombineBox->SetVisibility(false);
	this->m_CombineBox->bOnlyOwnerSee = true;

	//this->m_Recipes.Add({ 1.0f, EBlockType::Construction, EBlockType::Door, ATrapBlock::StaticClass() });
	//this->m_Recipes.Add({ 3.0f, EBlockType::Construction, EBlockType::Golden, AInfinityBlock::StaticClass() });
}

void USecondaryBrush::BeginPlay()
{
	Super::BeginPlay();

	// Set material for brush
	UMaterialInstanceDynamic *material = Super::GetMaterialDynamic();
	if (material != nullptr)
	{
		material->SetVectorParameterValue(BRUSH_COLOR_NAME, BRUSH_DESTROY_COLOR);
	}
}

void USecondaryBrush::SetMode(const EModifyMode& mode)
{
	if (this->m_Mode == mode)
	{
		return;
	}
	UMaterialInstanceDynamic *material = Super::GetMaterialDynamic();
	if (material == nullptr)
	{
		return;
	}
	this->m_Mode = mode;
	switch (this->m_Mode)
	{
		case EModifyMode::Destroy:
			material->SetVectorParameterValue(BRUSH_COLOR_NAME, BRUSH_DESTROY_COLOR);
			break;
		case EModifyMode::Combine:
			material->SetVectorParameterValue(BRUSH_COLOR_NAME, BRUSH_COMBINE_COLOR);
			break;
		default: break;
	}
}

ABlock* USecondaryBrush::Action(ABuildArea* area, AActor* source)
{
	if (!Super::IsBrushVisible())
	{
		return nullptr;
	}
	ABlock *block = Cast<ABlock>(Super::m_LastTrace.GetActor());
	if (block == nullptr || !block->IsDestructable() 
		|| (block->IsA(AFlagBlock::StaticClass()) && block->GetTeam() == area->GetTeam()))
	{
		return nullptr;
	}
	// If we get here, the destroy is valid and we will turn the block into a dropped one
	block->DropBlock(source, false);
	return block;
}

void USecondaryBrush::UpdateChain(ABuildArea *area)
{
	const FIntVector& last = Super::m_Chained.Last();
	if (!CELLS_EQUAL(Super::m_ActiveCell, last))
	{
		FIntVector temp = Super::m_ActiveCell;
		Super::m_ActiveCell = last;

		this->Action(area, nullptr);
		Super::m_ActiveCell = temp;

		Super::m_Chained.Add(Super::m_ActiveCell);
	}
}

void USecondaryBrush::UpdateCrafting(ABlock *block, UPrimaryBrush *primary, UBlockData *activeData)
{
	// If we were combining a block but then moved to another block, reset the combining
	if (this->m_CombineLock != nullptr && this->m_CombineLock != block)
	{
		*this->m_CraftTimer = 0.0f;
		this->m_bCombining = false;
		this->m_CombineLock = nullptr;
	}

	float duration = this->m_ActiveRecipe->m_Duration, elapsed = *this->m_CraftTimer - COMBINE_DEAD_TIME;
#if KOTC_CONSTRUCTION_QUICK_CRAFT
	duration = 0.25f;
#endif
	// If combining hasn't started yet but should
	if (!this->m_bCombining && elapsed >= 0.0f && elapsed < duration)
	{
		this->m_bCombining = true;
		this->m_CombineLock = block;
	}
	else if (this->m_bCombining)
	{
		// Update combine material
		UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_CombineBox->GetMaterial(0));
		if (material != nullptr)
		{
			material->SetScalarParameterValue(TEXT("Percentage"), elapsed / duration);

			if (!this->m_CombineBox->IsVisible())
			{
				this->m_CombineBox->SetVisibility(true);
			}
		}
		// If combining is complete
		if (elapsed >= duration)
		{
			this->m_bCombining = false;

			TArray<ABlock*> recipe;
			recipe.Add((ABlock*)block->GetClass()->GetDefaultObject());
			recipe.Add((ABlock*)activeData->GetClassType()->GetDefaultObject());

			FVector location = block->GetActorLocation();
			FRotator rotation = block->GetActorRotation();
			int team = block->GetTeam();

			block->DestroyBlock();
			ABlock *combined = ABlock::SpawnBlock(Super::GetWorld(),
				this->m_ActiveRecipe->m_ResultClassType, team, Super::GetOwner(), location, rotation);
			if (combined != nullptr)
			{
				//combined->SetRecipe(this->m_ActiveRecipe);
				combined->SetRecipe(recipe);
				activeData->SetCount(primary, activeData->GetCount() - 1);
			}
		}
	}
	if (!this->m_bCombining)
	{
		this->m_CombineBox->SetVisibility(false);
	}
}

void USecondaryBrush::Update(APlayerCharacter *character, ABuildArea* area, const FHitResult& trace)
{
	ABlock *block = Cast<ABlock>(trace.GetActor());
	if (block == nullptr)
	{
		Super::SetBrushVisible(false);
		return;
	}
	EModifyMode nextMode = EModifyMode::Destroy;
	bool combineValid = false;

	UPrimaryBrush *primary = character->GetPrimaryBrush();
	UBlockData *activeData = primary->GetBlockData(primary->GetSelectedIndex());
	if (activeData != nullptr && !Super::m_bChainMode)
	{
		const FName typeA = activeData->GetNameId(), typeB = block->GetNameId();
		for (const FBlockRecipe& recipe : this->m_Recipes)
		{
			if ((typeA == recipe.m_TypeA && typeB == recipe.m_TypeB)
				|| (typeA == recipe.m_TypeB && typeB == recipe.m_TypeA))
			{
				nextMode = EModifyMode::Combine;
				this->m_ActiveRecipe = &recipe;

				// Check to see if the player has enough blocks to make the combine
				combineValid = activeData->GetCount() > 0;
				break;
			}
		}
		Super::UpdateCountText(activeData);
	}
	if (this->GetMode() != nextMode)
	{
		this->SetMode(nextMode);
	}
	if (Super::m_bChainMode)
	{
		//this->UpdateChain(area);
	}
	Super::m_TextActor->SetActorHiddenInGame(primary->IsBrushVisible());

	if (Super::IsBrushVisible() && combineValid && this->GetMode() == EModifyMode::Combine
		&& this->m_CraftTimer != nullptr && this->m_ActiveRecipe != nullptr && !primary->IsChaining())
	{
		this->UpdateCrafting(block, primary, activeData);
	}
	else if (this->m_CombineBox->IsVisible())
	{
		this->m_CombineBox->SetVisibility(false);
	}

	// Check to see if cell is valid
	if (area->GetGridCell(block->GetActorLocation(), Super::m_ActiveCell))
	{
		//// Method 1 - Works with a single call but has jitter
		//FVector diff = Super::GetComponentLocation() - Super::Bounds.Origin;
		//Super::SetWorldLocation(block->GetActorLocation() + diff);
		//Super::SetWorldRotation(block->GetActorRotation());
		////

		// Method 2 - Works but has two calls
		Super::SetWorldLocation(block->GetActorLocation());
		Super::SetWorldRotation(block->GetActorRotation());

		FVector diff = block->GetActorLocation() - Super::Bounds.Origin;
		Super::SetWorldLocation(Super::GetComponentLocation() + diff);
		//

		Super::SetBrushVisible(true);
	}
	else
	{
		this->m_bCombining = false;
		Super::SetBrushVisible(false);
	}
	Super::Update(character, area, trace);
}
