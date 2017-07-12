#include "King_of_the_Castle.h"
#include "PrimaryBrush.h"

#include "Gamemode/BaseGameMode.h"
#include "Character/PlayerCharacter.h"
#include "Construction/Block.h"
#include "Construction/Prefab.h"
#include "Construction/BlockData.h"
#include "Construction/BuildArea.h"
#include "Construction/BlockEntity.h"

#include "DrawDebugHelpers.h"

#define DROP_STRENGTH_MAX 750.0f
#define DROP_STRENGTH_MIN 900.0f
#define DROP_ROTATION_OFFSET 100.0f //degrees

#define DEFAULT_BLOCK_COUNT 0
#define DEFAULT_MAX_BLOCK_COUNT 10
#define BRUSH_CREATE_COLOR FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)

#define BRUSH_TEXT_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_BillboardFont.M_BillboardFont'")

#define DATA_DOOR_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_DoorBlock_Data")
#define DATA_GOLD_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_GoldBlock_Data")
#define DATA_FLAG_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_FlagBlock_Data")
#define DATA_BASIC_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_BasicBlock_Data")

UPrimaryBrush::UPrimaryBrush() : m_bValid(false), m_SelectedTypeIndex(0)
{
	//Set default create brush blocks. Can be modified in blueprints.
	static ConstructorHelpers::FClassFinder<UBlockData> DataBasic(DATA_BASIC_LOCATION);
	if (DataBasic.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataBasic.Class);
	}

	static ConstructorHelpers::FClassFinder<UBlockData> DataDoor(DATA_DOOR_LOCATION);
	if (DataDoor.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataDoor.Class);
	}

	//static ConstructorHelpers::FClassFinder<UBlockData> DataGold(DATA_GOLD_LOCATION);
	//if (DataGold.Succeeded())
	//{
	//	this->m_BlockDataClasses.Add(DataGold.Class);
	//}

	static ConstructorHelpers::FClassFinder<UBlockData> DataFlag(DATA_FLAG_LOCATION);
	if (DataFlag.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataFlag.Class);
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

void UPrimaryBrush::DropBlocks(UBlockData* data, int count)
{
	AActor *parent = Super::GetOwner();
	if (data == nullptr || count <= 0 || parent == nullptr)
	{
		return;
	}
	count = FMath::Min(count, data->GetCount());

	ABaseGameMode *gamemode = Cast<ABaseGameMode>(parent->GetWorld()->GetAuthGameMode());
	// Drop count amount of blocks (limited to count of data - will not drop a block if it doesn't have it)
	for (int i = 0; i < count; i++)
	{
		data->SetCount(this, data->GetCount() - 1);

		for (ABlockEntity *next : ABlockEntity::SpawnBlockEntity((ABlock*)data->GetClassType()->GetDefaultObject(), parent->GetWorld(), nullptr, true))
		{
			next->SetBlockOwner(parent);
			next->SetIgnoreOwner(true);

			next->SetActorLocation(parent->GetActorLocation());
			next->SetActorRotation(parent->GetActorRotation());

			FVector rotation = parent->GetActorRotation().Vector();
			((UPrimitiveComponent*)next->GetRootComponent())->AddImpulse(rotation
				* FMath::FRandRange(DROP_STRENGTH_MIN, DROP_STRENGTH_MAX));

			if (gamemode != nullptr)
			{
				gamemode->OnBlockDrop(next, Cast<APlayerCharacter>(parent), data->GetCount());
			}
		}
	}
}

void UPrimaryBrush::UpdateBlockChild()
{
	for(AActor *actor : this->m_ChildBlocks)
	{
		actor->Destroy();
	}
	this->m_ChildBlocks.Empty();

	auto addChild = [this](UBlockData *data, FVector offset = FVector::ZeroVector)
	{
		if(data == nullptr || Super::GetWorld() == nullptr)
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
			block->SetActorRelativeLocation(FVector(0.0f, -Super::Bounds.BoxExtent.Y, 0.0f) + offset);
			block->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

			this->m_ChildBlocks.Add(block);
		}
	};

	if(this->m_Prefab != nullptr)
	{
		APrefab *prefab = Super::GetWorld()->SpawnActor<APrefab>(this->m_Prefab);
		if(prefab == nullptr)
		{
			return;
		}
		prefab->SetActorHiddenInGame(true);

		for(const FPrefabBlock& pb : prefab->GetBlocks())
		{
			addChild(this->GetBlockData(pb.id), FVector(pb.offset) * Super::Bounds.BoxExtent * 2.0f);
		}
		prefab->Destroy();
	}
	else
	{
		addChild(this->m_BlockData[this->m_SelectedTypeIndex]);
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
	// If chaining, ensure there's a blocking object below where we're trying to place
	//if (Super::m_bChainMode&&false)
	//{
	//	FVector location;
	//	area->GetGridLocation(Super::m_ActiveCell, location);

	//	FHitResult result;
	//	Super::GetWorld()->LineTraceSingleByChannel(result, location, location - FVector(0.0f, 0.0f, area->GetCellSize().Z),
	//		ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
	//	if (!result.IsValidBlockingHit())
	//	{
	//		return nullptr;
	//	}
	//}
	TArray<ABlock*> placed;
	for(ABlock *block : this->m_ChildBlocks)
	{
		UBlockData *data = this->GetBlockData(block->GetNameId());
		if (data == nullptr || data->GetCount() <= 0)
		{
			continue;
		}
		FIntVector cell;
		area->GetGridCell(block->GetActorLocation(), cell);

		ABlock *actor = area->SpawnBlockAt(cell, data->GetClassType(), Cast<APlayerCharacter>(Super::GetOwner()));
		if (actor != nullptr)
		{
			// Spawn the block in the air slightly so it drops
			FVector position = actor->GetActorLocation();
			position.Z += BRUSH_SPAWN_Z_OFFSET;
			actor->SetActorLocation(position);

			data->SetCount(this, data->GetCount() - 1);

			placed.Add(actor);
		}
	}
	//TODO
	return placed.Num() > 0 ? placed[0] : nullptr;

	//UBlockData *data = this->GetBlockData(this->m_SelectedTypeIndex);
	//if (data == nullptr || data->GetCount() <= 0)
	//{
	//	return nullptr; //none of this block remaining to place
	//}
	//// If everything is fine and we should try to spawn
	//ABlock *actor = area->SpawnBlockAt(Super::m_ActiveCell, data->GetClassType(), Super::GetOwner());
	//if (actor != nullptr)
	//{
	//	// Spawn the block in the air slightly so it drops
	//	FVector position = actor->GetActorLocation();
	//	position.Z += BRUSH_SPAWN_Z_OFFSET;
	//	actor->SetActorLocation(position);

	//	data->SetCount(this, data->GetCount() - 1);
	//}
	//return actor;
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
#if WITH_EDITOR
	if (this->m_bDebugRenderTrace)
	{
		if (trace.GetComponent() == nullptr)
		{
			DrawDebugLine(Super::GetWorld(), trace.TraceStart, trace.TraceEnd, FColor::Red, false, -1.0f, 0.0f, 2.0f);
		}
		else
		{
			DrawDebugLine(Super::GetWorld(), trace.TraceStart, trace.ImpactPoint, FColor::Green, false, -1.0f, 0.0f, 2.0f);
			DrawDebugLine(Super::GetWorld(), trace.ImpactPoint, trace.TraceEnd, FColor::Red, false, -1.0f, 0.0f, 2.0f);
		}
	}
#endif
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
#if WITH_EDITOR
			if (this->m_bDebugRenderTrace)
			{
				DrawDebugLine(Super::GetWorld(), belowResult.TraceStart, belowResult.TraceEnd, FColor::Magenta, false, -1.0f, 0.0f, 2.0f);
			}
#endif

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
		FVector normal = (trace.TraceEnd - trace.TraceStart).GetSafeNormal();
		float distance = diff.Size();
		diff = diff.GetSafeNormal() * area->GetCellSize() * 0.5f;

		AActor *below = nullptr;
		while (FMath::FloorToFloat(distance) > 0.0f)
		{
			point += diff;
			distance -= diff.Size();

			FIntVector cell;
			if (area->GetGridCell(point, cell))
			{
				FHitResult result;
				// If we're looking downwards cast the sideways way else cast the downwards one
				if (normal.Z <= -0.75f)
				{
					FVector sideDir = FVector(normal.X, normal.Y, 0.0f).GetSafeNormal();
					Super::GetWorld()->LineTraceSingleByChannel(result, point, point - sideDir * area->GetCellSize().Z,
						ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
#if WITH_EDITOR
					if (this->m_bDebugRenderTrace)
					{
						if (result.IsValidBlockingHit())
						{
							DrawDebugLine(Super::GetWorld(), result.TraceStart, result.ImpactPoint, FColor::Cyan, false, -1.0f, 0.0f, 2.0f);
						}
						else
						{
							DrawDebugLine(Super::GetWorld(), result.TraceStart, result.TraceEnd, FColor::Blue, false, -1.0f, 0.0f, 2.0f);
						}
					}
#endif
				}
				else
				{
					// Cast a small ray down from the current cell to check if there's something below
					Super::GetWorld()->LineTraceSingleByChannel(result, point, point - FVector(0.0f, 0.0f, area->GetCellSize().Z),
						ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
#if WITH_EDITOR
					if (this->m_bDebugRenderTrace)
					{
						if (result.IsValidBlockingHit())
						{
							DrawDebugLine(Super::GetWorld(), result.TraceStart, result.ImpactPoint, FColor::Cyan, false, -1.0f, 0.0f, 2.0f);
						}
						else
						{
							DrawDebugLine(Super::GetWorld(), result.TraceStart, result.TraceEnd, FColor::Blue, false, -1.0f, 0.0f, 2.0f);
						}
					}
#endif
				}
				// Check to see if there's a block below
				ABlock *block = Cast<ABlock>(result.GetActor());
				if (block != nullptr)
				{
					// Check to see if the block below is visible to the player
					Super::GetWorld()->LineTraceSingleByChannel(result, trace.TraceStart, result.ImpactPoint - result.ImpactNormal,
						ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);
#if WITH_EDITOR
					if (this->m_bDebugRenderTrace)
					{
						if (result.GetActor() == block)
						{
							DrawDebugLine(Super::GetWorld(), result.TraceStart, result.ImpactPoint, FColor::Orange, false, -1.0f, 0.0f, 2.0f);
						}
						else
						{
							DrawDebugLine(Super::GetWorld(), result.TraceStart, result.TraceEnd, FColor::Magenta, false, -1.0f, 0.0f, 2.0f);
						}
					}
#endif
					if(result.GetActor() == block)
					{
						below = block;

						show = true;
						this->m_bValid = true;
						Super::m_ActiveCell = cell;
					}
				}
				else if (below != nullptr)
				{
					break;
				}
			}
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
	for(AActor *child : this->m_ChildBlocks)
	{
		child->SetActorHiddenInGame(!Super::IsBrushVisible());
	}

	Super::Update(character, area, trace);
}