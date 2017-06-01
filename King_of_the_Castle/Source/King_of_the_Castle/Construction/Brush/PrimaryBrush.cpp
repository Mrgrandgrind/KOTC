#include "King_of_the_Castle.h"
#include "PrimaryBrush.h"

#include "../Block.h"
#include "../BlockData.h"
#include "../BuildArea.h"
#include "../BlockEntity.h"

#define DEFAULT_BLOCK_COUNT 0
#define DEFAULT_MAX_BLOCK_COUNT 10
#define BRUSH_CREATE_COLOR FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)

#define BRUSH_SPAWN_Z_OFFSET 16.0f // How far up from desired location to spawn a block when placed (so it falls into place)

#define BRUSH_TEXT_MATERIAL_LOCATION TEXT("Material'/Game/ThirdPersonBP/Materials/M_BillboardFont.M_BillboardFont'")

#define DATA_DOOR_LOCATION TEXT("/Game/ThirdPersonBP/Blueprints/Construction/BlockData/BP_Data_DoorBlock")
#define DATA_GOLD_LOCATION TEXT("/Game/ThirdPersonBP/Blueprints/Construction/BlockData/BP_Data_GoldBlock")
#define DATA_CONSTRUCTION_LOCATION TEXT("/Game/ThirdPersonBP/Blueprints/Construction/BlockData/BP_Data_ConstructionBlock")

UPrimaryBrush::UPrimaryBrush() : m_bValid(false), m_Child(nullptr), m_SelectedTypeIndex(0)
{
	//Set default create brush blocks. Can be modified in blueprints.
	static ConstructorHelpers::FClassFinder<UBlockData> DataConstruction(DATA_CONSTRUCTION_LOCATION);
	if (DataConstruction.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataConstruction.Class);
	}

	static ConstructorHelpers::FClassFinder<UBlockData> DataDoor(DATA_DOOR_LOCATION);
	if (DataDoor.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataDoor.Class);
	}

	static ConstructorHelpers::FClassFinder<UBlockData> DataGold(DATA_GOLD_LOCATION);
	if (DataGold.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataGold.Class);
	}

	Super::bAbsoluteRotation = true;
}

void UPrimaryBrush::BeginPlay()
{
	// Call super begin player
	Super::BeginPlay();

	// Set block data for the brush
	this->m_BlockData.Empty();
	for (const TSubclassOf<UBlockData>& type : this->m_BlockDataClasses)
	{
		this->m_BlockData.Add(NewObject<UBlockData>(this, type));
	}
	this->SetSelectedIndex(0);

	// Set material for brush
	UMaterialInstanceDynamic *material = Super::GetMaterialDynamic();
	if (material != nullptr)
	{
		material->SetVectorParameterValue(BRUSH_COLOR_NAME, BRUSH_CREATE_COLOR);
	}
}

UBlockData* UPrimaryBrush::GetBlockData(const int& index)
{
	if (index < 0 || index >= this->m_BlockData.Num())
	{
		return nullptr;
	}
	return this->m_BlockData[index];
}

int UPrimaryBrush::GetIndexOf(const FName id) const
{
	for (int i = 0; i < this->m_BlockData.Num(); i++)
	{
		if (this->m_BlockData[i] == nullptr)
		{
			continue;
		}
		if (this->m_BlockData[i]->GetNameId() == id)
		{
			return i;
		}
	}
	return -1;
}

void UPrimaryBrush::SetSelectedIndex(int index)
{
	if (index < 0)
	{
		index = this->m_BlockData.Num() - 1;
	}
	else if (index >= this->m_BlockData.Num())
	{
		index = 0;
	}
	this->m_SelectedTypeIndex = index;

	this->UpdateBlockChild();
	Super::UpdateCountText(this->GetBlockData(index));
}

void UPrimaryBrush::UpdateBlockChild()
{
	if (this->m_Child != nullptr)
	{
		this->m_Child->Destroy();
		this->m_Child = nullptr;
	}
	UBlockData *data = this->m_BlockData[this->m_SelectedTypeIndex];
	if (data == nullptr || Super::GetWorld() == nullptr)
	{
		return;
	}
	ABlock *block = Super::GetWorld()->SpawnActor<ABlock>(data->GetClassType());
	if (block != nullptr)
	{
		UStaticMeshComponent *mesh = block->GetMesh();
		mesh->SetSimulatePhysics(false);
		mesh->bGenerateOverlapEvents = false;
		mesh->SetWorldScale3D(mesh->GetComponentScale() * 0.975f); //magic number. used to make the block fit inside the brush.
		mesh->bOnlyOwnerSee = true;

		if (data->GetGhostMaterial() != nullptr)
		{
			mesh->SetMaterial(0, data->GetGhostMaterial());
		}
		if (Super::m_Team != nullptr)
		{
			block->SetTeam(*Super::m_Team);
		}
		mesh->SetCollisionProfileName(TEXT("NoCollision"));
		mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		block->SetOwner(Super::GetAttachmentRootActor());
		block->SetActorHiddenInGame(!Super::IsBrushVisible());
		block->SetActorRelativeLocation(FVector(0.0f, -Super::Bounds.BoxExtent.Y, 0.0f));
		block->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

		this->m_Child = block;
	}
}

ABlock* UPrimaryBrush::Action(ABuildArea* area, AActor* source)
{
	if (!this->m_bValid)
	{
		return nullptr; //brush is inactive
	}
	if (!Super::m_bChainMode && Super::m_Chained.Num() > 0)
	{
		return nullptr;
	}
	UBlockData *data = this->GetBlockData(this->m_SelectedTypeIndex);
	if (data == nullptr || data->GetCount() <= 0)
	{
		return nullptr; //none of this block remaining to place
	}
	// If chaining, ensure there's a blocking object below where we're trying to place
	if (Super::m_bChainMode)
	{
		FVector location;
		area->GetGridLocation(Super::m_ActiveCell, location);

		FHitResult result;
		Super::GetWorld()->LineTraceSingleByChannel(result, location, location - FVector(0.0f, 0.0f, area->GetCellSize().Z),
			ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
		if (!result.IsValidBlockingHit())
		{
			return nullptr;
		}
	}
	// If everything is fine and we should try to spawn
	ABlock *actor = area->SpawnBlockAt(Super::m_ActiveCell, data->GetClassType());
	if (actor != nullptr)
	{
		// Spawn the block in the air slightly so it drops
		FVector position = actor->GetActorLocation();
		position.Z += BRUSH_SPAWN_Z_OFFSET;
		actor->SetActorLocation(position);

		data->SetCount(this, data->GetCount() - 1);
	}
	return actor;
}

void UPrimaryBrush::UpdateChain(ABuildArea* area, const FHitResult& trace, bool& show)
{
	const FIntVector& last = Super::m_Chained.Last();
	bool place = false;

	FVector end = trace.IsValidBlockingHit() ? trace.ImpactPoint : trace.TraceEnd;
	FVector vector = trace.TraceStart - end;
	FVector point = end, diff = area->GetCellSize() * 0.25f * vector.GetSafeNormal();

	// Get rotation of trace
	FRotator rotation;
#if KOTC_CONSTRUCTION_CHAIN_AXIS_LOCK
	if (Super::m_Chained.Num() >= 2)
	{
		rotation = Super::m_ChainedRotation;
	}
	else
#endif
	{
		rotation = vector.Rotation();
		rotation.Pitch = rotation.Roll = 0.0f;
		rotation.Yaw = int(FMath::GridSnap(rotation.Yaw, 90.0f)) % 360;
	}
	float distance = vector.Size();
	while (distance > 0.0f)
	{
		FIntVector check;
		area->GetGridCell(point, check);

		if (CELLS_EQUAL(check, last))
		{
			show = true;
			place = false;
			this->m_bValid = true;
			Super::m_ActiveCell = check;
			break;
		}
		// If the cell is next to last and that the cell is not below last (we don't want to place block below others because of physics)
		if (CELLS_BESIDE(check, last) && !CELLS_EQUAL(check, FIntVector(last.X, last.Y, last.Z - 1)))
		{
			// Check to see if the cell is not the one infront or behind of last
			FVector rotate = rotation.Vector();
			FIntVector towardsCell(last.X + FMath::RoundToInt(rotate.X), last.Y + FMath::RoundToInt(rotate.Y), last.Z + FMath::RoundToInt(rotate.Z));
			FIntVector awayCell(last.X - FMath::RoundToInt(rotate.X), last.Y - FMath::RoundToInt(rotate.Y), last.Z - FMath::RoundToInt(rotate.Z));
			if (!CELLS_EQUAL(check, towardsCell) && !CELLS_EQUAL(check, awayCell))
			{
				FVector lastLoc;
				area->GetGridLocation(last, lastLoc);

				FHitResult lastResult;
				Super::GetWorld()->LineTraceSingleByChannel(lastResult, lastLoc, lastLoc - FVector(0.0f, 0.0f, area->GetCellSize().Z),
					ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);

				show = true;
				place = lastResult.IsValidBlockingHit();
				this->m_bValid = lastResult.IsValidBlockingHit();
				Super::m_ActiveCell = check;
			}
		}

		point += diff;
		distance -= diff.Size();
	}
	// Check for overlaps with brush. If there is, add the overlap to our chain and do nothing else
	Super::SetPositionToCell(area, Super::m_ActiveCell);
	if (Super::IsOverlapped())
	{
		// Only need to add it if it's not already part of the chain
		if (!CELLS_EQUAL(Super::m_ActiveCell, last))
		{
			Super::m_Chained.Add(Super::m_ActiveCell);
		}
		return;
	}
	if (place)
	{
		Super::SetPositionToCell(area, last);
		if (!Super::IsOverlapped())
		{
			FIntVector temp = Super::m_ActiveCell;
			Super::m_ActiveCell = last;

			this->Action(area, nullptr);
			Super::m_ActiveCell = temp;
		}
		Super::m_Chained.Add(Super::m_ActiveCell);

#if KOTC_CONSTRUCTION_CHAIN_AXIS_LOCK
		// After initial chain blocked has been placed, store the used rotation so that we can lock
		// the axis which the chian will work on
		if (Super::m_Chained.Num() == 2)
		{
			Super::m_ChainedRotation = rotation;
		}
#endif
	}
	else
	{
		Super::SetPositionToCell(area, last);
	}
}

void UPrimaryBrush::UpdateRegular(ABuildArea* area, const FHitResult& trace, bool& show)
{
	// If we are aiming at something
	if (trace.GetComponent() != nullptr)
	{
		// Check to see if the new point is a valid grid cell
		show = area->GetGridCell(trace.ImpactPoint + trace.ImpactNormal * area->GetCellSize() / 2.0f, Super::m_ActiveCell);
		this->m_bValid = show;

		// Only show create brush if looking at top of block
		if (show && FMath::RoundToInt(trace.ImpactNormal.Z) != 1)
		{
			FHitResult belowResult;
			FVector point = trace.ImpactPoint + trace.ImpactNormal * area->GetCellSize() / 2.0f;
			Super::GetWorld()->LineTraceSingleByChannel(belowResult, point, point - FVector(0.0f, 0.0f, area->GetCellSize().Z),
				ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);

			if (Cast<ABlock>(belowResult.GetActor()) == nullptr)
			{
				show = false;
			}
		}
	}

	// If we aren't aiming at something, or if we are aiming at something that's not a block
	if (!this->m_bValid || Cast<ABlock>(trace.GetActor()) == nullptr)
	{
		// Trace the ray to determine if there are any blocks below the grid elements we are looking above
		FVector point = trace.TraceStart, diff = (trace.IsValidBlockingHit()
			? trace.ImpactPoint : trace.TraceEnd) - trace.TraceStart;
		float distance = diff.Size();
		diff = diff.GetSafeNormal() * area->GetCellSize() * 0.25f;

		AActor *below = nullptr;
		while (distance > 0)
		{
			FIntVector cell;
			if (area->GetGridCell(point, cell))
			{
				// Cast a small ray down from the current cell to check if there's something below
				FHitResult belowResult;
				Super::GetWorld()->LineTraceSingleByChannel(belowResult, point, point - FVector(0.0f, 0.0f, area->GetCellSize().Z),
					ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);

				// Check to see if there's a block below
				ABlock *block = Cast<ABlock>(belowResult.GetActor());
				if (block != nullptr)
				{
					below = block;

					show = true;
					this->m_bValid = true;
					Super::m_ActiveCell = cell;
				}
				else if (below != nullptr)
				{
					break;
				}
			}
			point += diff;
			distance -= diff.Size();
		}
	}
	// If the brush is valid, move it to the expected location
	if (this->m_bValid)
	{
		Super::SetPositionToCell(area, Super::m_ActiveCell);
	}
}

void UPrimaryBrush::Update(APlayerCharacter *character, ABuildArea* area, const FHitResult& trace)
{
	checkf(area != nullptr, TEXT("[CreateBrush] Must provide area"));

	bool show = false;
	this->m_bValid = false;

#if KOTC_CONSTRUCTION_CHAIN_ENABLED
	if (Super::m_bChainMode)
	{
		this->UpdateChain(area, trace, show);
	}
	else
#endif
	{
		this->UpdateRegular(area, trace, show);
	}

	bool overlapped = Super::IsOverlapped();
	// Creation can't be valid if there is an overlap (there's technically a one-update error here since it checks before setting the position)
	this->m_bValid = this->m_bValid && !overlapped;

	// Set brush visiblity
	Super::SetBrushVisible(show && !overlapped);

	// TODO visibility hack. do it properly
	if (this->m_Child != nullptr)
	{
		this->m_Child->SetActorHiddenInGame(!Super::IsBrushVisible());
	}

	Super::Update(character, area, trace);
}