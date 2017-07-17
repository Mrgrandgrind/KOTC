#include "King_of_the_Castle.h"
#include "PrimaryBrush.h"

#include "Gamemode/BaseGameMode.h"
#include "Construction/Block.h"
#include "Construction/Prefab.h"
#include "Construction/BlockData.h"
#include "Construction/BuildArea.h"
#include "Construction/BlockEntity.h"
#include "Character/PlayerCharacter.h"

#include "Runtime/Engine/Classes/Components/ModelComponent.h"

#define DROP_STRENGTH_MAX 750.0f
#define DROP_STRENGTH_MIN 900.0f
#define DROP_ROTATION_OFFSET 100.0f //degrees

#define DEFAULT_BLOCK_COUNT 0
#define DEFAULT_MAX_BLOCK_COUNT 10
#define BRUSH_CREATE_COLOR FLinearColor(0.0f, 0.75f, 0.0f, 1.0f)

#define BRUSH_PLACE_FLOOR_NORMAL_Z -0.5f

#define BRUSH_TEXT_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_BillboardFont.M_BillboardFont'")

#define DATA_DOOR_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_DoorBlock_Data")
#define DATA_GOLD_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_GoldBlock_Data")
#define DATA_FLAG_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_FlagBlock_Data")
#define DATA_BASIC_LOCATION TEXT("/Game/Blueprints/Construction/BrushData/BP_BasicBlock_Data")

UPrimaryBrush::UPrimaryBrush() : m_SelectedTypeIndex(0)
{
	//Set default create brush blocks. Can be modified in blueprints.
	static ConstructorHelpers::FClassFinder<UBlockData> DataBasic(DATA_BASIC_LOCATION);
	if (DataBasic.Succeeded())
	{
		this->m_BlockDataClasses.Add(DataBasic.Class);
	}

	//static ConstructorHelpers::FClassFinder<UBlockData> DataDoor(DATA_DOOR_LOCATION);
	//if (DataDoor.Succeeded())
	//{
	//	this->m_BlockDataClasses.Add(DataDoor.Class);
	//}

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

	// Set material for brush
	UMaterialInstanceDynamic *material = Super::GetMaterialDynamic();
	if (material != nullptr)
	{
		material->SetVectorParameterValue(BRUSH_COLOR_NAME, BRUSH_CREATE_COLOR);
	}
	this->SetSelectedIndex(0);
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

	this->UpdateChildBlocks();
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

FRotator UPrimaryBrush::GetBrushRotation() const
{
	APlayerCharacter *character = Cast<APlayerCharacter>(Super::GetOwner());
	FRotator rotation = (Super::GetComponentLocation()
		- character->GetCamera()->GetComponentLocation()).Rotation();
	rotation.Pitch = FMath::GridSnap(rotation.Pitch, 90.0f);
	rotation.Yaw = FMath::GridSnap(rotation.Yaw, 90.0f) + 90.0f;
	rotation.Roll = 0.0f;
	return rotation;
}

bool UPrimaryBrush::CanPlaceOn(const FHitResult& result) const
{
	return result.IsValidBlockingHit()
		&& Cast<UStaticMeshComponent>(result.GetComponent()) != nullptr
		|| result.GetComponent()->IsA(UModelComponent::StaticClass());
}

void UPrimaryBrush::SetBrushVisible(const bool& visible)
{
	for (AActor *actor : this->m_ChildBlocks)
	{
		actor->SetActorHiddenInGame(!visible);
	}
	Super::SetBrushVisible(visible);
}

void UPrimaryBrush::UpdateChildBlocks()
{
	for (AActor *next : this->m_ChildBlocks)
	{
		next->Destroy();
	}
	this->m_ChildBlocks.Empty();

	UBlockData *data = this->GetBlockData(this->GetSelectedIndex());
	if (data == nullptr)
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

		this->m_ChildBlocks.Add(block);
	}
}

TArray<ABlock*> UPrimaryBrush::OnAction(ABuildArea *area, AActor *source)
{
	TArray<ABlock*> blocks;
	if (!Super::IsPositionValid())
	{
		return blocks;
	}
	// Verify the player has enough blocks to place
	TMap<FName, int> requirements;
	for (ABlock *block : this->m_ChildBlocks)
	{
		if (requirements.Contains(block->GetNameId()))
		{
			requirements[block->GetNameId()]++;
		}
		else
		{
			requirements.Add(block->GetNameId(), 1);
		}
	}
	for (auto entry : requirements)
	{
		UBlockData *data = this->GetBlockData(entry.Key);
		if (data == nullptr || data->GetCount() < entry.Value)
		{
			return blocks;
		}
	}

	TArray<ABlock*> placed;
	for(ABlock *block : this->m_ChildBlocks)
	{
		UBlockData *data = this->GetBlockData(block->GetNameId());

		// Check to see if we are overlapping with another block.
		// GetOverlappingActors does not work because of collision setup. We will use line trace.
		FHitResult result;
		FVector end = block->GetActorLocation(), start = end + FVector(0.0f, 0.0f, Super::Bounds.BoxExtent.Z + 4.0f);
		Super::GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_WorldDynamic);

		if (result.IsValidBlockingHit() && result.GetActor() != nullptr)
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
	return blocks;
}

bool UPrimaryBrush::OnPreCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show)
{
	// We will use the precheck to prcoess when we have hit something with our ray
	if (result.IsValidBlockingHit())
	{
		check(result.GetComponent() != nullptr);

		// If we are looking at a static mesh component or model component
		if (this->CanPlaceOn(result))
		{
			// The position is valid so update the position into our out cell
			area->GetGridCell(result.ImpactPoint + result.ImpactNormal * area->GetCellSize() / 2.0f, out);

			// Check to see if we new position would spawn a support block,
			// if it does then we will always show the brush
			FVector location;
			area->GetGridLocation(out, location);

			show = Super::IsSupport(location, area->GetCellSize());

			// If we aren't in a support block spawning location but our ray hit a block
			//ABlock *block = Cast<ABlock>(result.GetActor());
			if (!show && result.GetComponent() != nullptr)
			{
				// Check to see if there's a block below the cell. If there is, we will still show
				FHitResult below;
				Super::GetWorld()->LineTraceSingleByChannel(below, location, location
					- FVector(0.0f, 0.0f, area->GetCellSize().Z), ECollisionChannel::ECC_WorldDynamic);
				show = below.IsValidBlockingHit() && Cast<ABlock>(below.GetActor()) != nullptr;

				Super::RenderTrace(below.TraceStart, show ? below.ImpactPoint : below.TraceEnd, show ? FColor::Cyan : FColor::Blue);
			}
			return true;
		}
	}
	return false;
}

bool UPrimaryBrush::OnMainCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& pre)
{
	// If we get to here it means that our trace did not hit anything
	const FVector& start = result.TraceStart, end = result.IsValidBlockingHit() ? result.ImpactPoint : result.TraceEnd;
	FVector vector = end - start, normal = vector.GetSafeNormal();
	if (pre && normal.Z > BRUSH_PLACE_FLOOR_NORMAL_Z)
	{
		// We only need to run main if pre failed or we are looking downwards
		return true;
	}
	float d = 0.0f, dTotal = vector.Size(), dStep = (area->GetCellSize() * normal).Size() * (1.0f / 1.5f);
	while (d < dTotal)
	{
		FVector point;
		FHitResult trace;

		// If the player is looking downwards
		if (normal.Z <= BRUSH_PLACE_FLOOR_NORMAL_Z)
		{
			// Trace from start to end
			point = start + d * normal;

			// Cast a ray towards the player to see if we can place a block on the floor
			FVector sideDir = FVector(normal.X, normal.Y, 0.0f).GetSafeNormal();
			Super::GetWorld()->LineTraceSingleByChannel(trace, point, point - sideDir * area->GetCellSize().Z,
				ECollisionChannel::ECC_WorldDynamic, FCollisionQueryParams::DefaultQueryParam);

			bool hit = trace.IsValidBlockingHit() && this->CanPlaceOn(trace);
			Super::RenderTrace(trace.TraceStart, hit ? trace.ImpactPoint : trace.TraceEnd, hit ? FColor::Cyan : FColor::Blue);
		}
		else
		{
			// Trace from end to start
			point = end - d * normal;

			// Cast a block size trace downward from the original trace
			Super::GetWorld()->LineTraceSingleByChannel(trace, point, point
				- FVector(0.0f, 0.0f, area->GetCellSize().Z), ECollisionChannel::ECC_WorldDynamic);

			bool hit = trace.IsValidBlockingHit() && this->CanPlaceOn(trace);
			Super::RenderTrace(trace.TraceStart, hit ? trace.ImpactPoint : trace.TraceEnd, hit ? FColor::Cyan : FColor::Blue);
		}
		// If this point is a valid place for a block to go
		if (trace.IsValidBlockingHit() && this->CanPlaceOn(trace))
		{
			// Do an additional check so that when placing on a ledge, it will choose the closest
			// instead of the furthest. This simply makes sure that we can see the block we hit.
			FCollisionQueryParams params;
			params.AddIgnoredActor(Super::GetOwner()); // Awkward camera angles can cause us to hit the player, so exclude it

			FHitResult traceCheck;
			Super::GetWorld()->LineTraceSingleByChannel(traceCheck, start,
				trace.ImpactPoint - trace.ImpactNormal, ECollisionChannel::ECC_WorldDynamic, params);

			// If both or rays hit the same thing then we know this position is actually valid
			bool valid = (trace.GetActor() != nullptr && traceCheck.GetActor() == trace.GetActor())
				|| (trace.GetComponent() != nullptr && traceCheck.GetComponent() == trace.GetComponent());

			Super::RenderTrace(traceCheck.TraceStart, valid ? traceCheck.ImpactPoint
				: traceCheck.TraceEnd, valid ? FColor::Cyan : FColor::Red);

			if (valid)
			{
				FVector pointB = trace.ImpactPoint + (trace.TraceEnd - trace.TraceStart)
					.GetSafeNormal() * area->GetCellSize() / 2.0f;
				FGridCell tempA, tempB;
				area->GetGridCell(point, tempA);
				area->GetGridCell(pointB, tempB);

				Super::RenderPoint(pointB, FColor::Purple);

				if (CELLS_BESIDE(tempA, tempB) || CELLS_EQUAL(tempA, tempB))
				{
					out = tempA; // This is necessary so we don't override pre check
					return show = true;
				}
				else
				{
					Super::RenderPoint(point, FColor::Red);
				}
			}
			else
			{
				Super::RenderPoint(point, FColor::Red);
			}
		}
		d += dStep;
	}
	return pre;
}

bool UPrimaryBrush::OnPostCheck(ABuildArea *area, const FHitResult& result, FGridCell& out, bool& show, const bool& valid)
{
	if (!valid)
	{
		return false;
	}
	bool overlap = Super::IsOverlapped();
	show = show && !overlap;
	return !overlap;
}