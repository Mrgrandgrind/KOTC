// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "BuildArea.h"

#include "Block.h"
#include "../Gamemode/BaseGameMode.h"
#include "../Character/PlayerCharacter.h"

#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Public/EngineUtils.h"

ABuildArea::ABuildArea() : m_Team(-1), m_CellSize(1.0f)
{
#if WITH_EDITOR
	Super::PrimaryActorTick.bCanEverTick = true;
#endif

	this->m_Area = UObject::CreateDefaultSubobject<UBoxComponent>(TEXT("BoxArea"));
	this->m_Area->SetCollisionProfileName(TEXT("OverlapAll"));
	this->m_Area->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TScriptDelegate<FWeakObjectPtr> delegate;
	delegate.BindUFunction(this, FName("OnBeginOverlap"));
	this->m_Area->OnComponentBeginOverlap.Add(delegate);

	Super::RootComponent = this->m_Area;
}

void ABuildArea::BeginPlay()
{
	Super::BeginPlay();

	// Ensure that the grid size are up-to-date with the cell size
	this->CalculateGridSize();

	if (Super::GetWorld() == nullptr)
	{
		return;
	}
	ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
	if (gamemode == nullptr)
	{
		return;
	}
	// Add the score for all existing blocks
	FVector origin, extent;
	Super::GetActorBounds(false, origin, extent);
	FBox bounds = FBox(origin - extent, origin + extent);
	for (TActorIterator<ABlock> itr(Super::GetWorld()); itr; ++itr)
	{
		if (bounds.IsInside(itr->GetActorLocation()))
		{
			gamemode->OnBlockPlace(*itr, this);
		}
	}
}

void ABuildArea::Tick(float delta)
{
	Super::Tick(delta);

#if WITH_EDITOR
	if (!this->m_DebugRenderGrid)
	{
		return;
	}
	// Render the grid using debug lines
	FVector corner = this->m_Area->Bounds.Origin - this->m_Area->Bounds.BoxExtent;
	for (int k = 0; k <= this->m_GridSize.Z; k++)
	{
		for (int j = 0; j <= this->m_GridSize.Y; j++)
		{
			FVector start(0.0f, j * this->m_CellSize.Y, k * this->m_CellSize.Z);
			FVector end(this->m_CellSize.X * this->m_GridSize.X, j * this->m_CellSize.Y, k * this->m_CellSize.Z);
			DrawDebugLine(Super::GetWorld(), start + corner, end + corner, FColor::Emerald, false, -1.0f, 0.0f, 2.0f);

			for (int i = 0; i <= this->m_GridSize.X; i++)
			{
				DrawDebugLine(Super::GetWorld(),
					corner + FVector(i * this->m_CellSize.X, 0.0f, k * this->m_CellSize.Z),
					corner + FVector(i * this->m_CellSize.X, this->m_CellSize.Y * this->m_GridSize.Y, k * this->m_CellSize.Z),
					FColor::Emerald, false, -1.0f, 0.0f, 2.0f);

				DrawDebugLine(Super::GetWorld(),
					corner + FVector(i * this->m_CellSize.X, j * this->m_CellSize.Y, 0.0f),
					corner + FVector(i * this->m_CellSize.X, j * this->m_CellSize.Y, this->m_GridSize.Z * this->m_CellSize.Z),
					FColor::Emerald, false, -1.0f, 0.0f, 2.0f);
			}
		}
	}
#endif
}

void ABuildArea::CalculateGridSize()
{
	if (this->m_CellSize.X <= 0.0f || this->m_CellSize.Y <= 0.0f || this->m_CellSize.Z <= 0.0f)
	{
		UE_LOG(LogClass, Error, TEXT("Cell size must not have a zero or negative component"));
		return;
	}
	const FVector& extent = this->m_Area->Bounds.BoxExtent;
	this->m_GridSize.X = int(extent.X * 2.0f / this->m_CellSize.X);
	this->m_GridSize.Y = int(extent.Y * 2.0f / this->m_CellSize.Y);
	this->m_GridSize.Z = int(extent.Z * 2.0f / this->m_CellSize.Z);
}

bool ABuildArea::IsCellValid(const FIntVector& cell) const
{
	return cell.X >= 0 && cell.X < this->m_GridSize.X
		&& cell.Y >= 0 && cell.Y < this->m_GridSize.Y
		&& cell.Z >= 0 && cell.Z < this->m_GridSize.Z;
}

bool ABuildArea::GetGridCell(const FVector& location, FIntVector& out) const
{
	if (this->m_CellSize.X <= 0.0f || this->m_CellSize.Y <= 0.0f || this->m_CellSize.Z <= 0.0f)
	{
		UE_LOG(LogClass, Error, TEXT("Cell size must not have a zero or negative component"));
		return false;
	}
	const FVector& origin = this->m_Area->Bounds.Origin;
	const FVector& extent = this->m_Area->Bounds.BoxExtent;
	FVector gridPosition = (location - origin + extent) / this->m_CellSize;
	out.X = int(gridPosition.X);
	out.Y = int(gridPosition.Y);
	out.Z = int(gridPosition.Z);
	return this->IsCellValid(out);
}

bool ABuildArea::GetGridLocation(const FIntVector& cell, FVector& out) const
{
	if (!this->IsCellValid(cell))
	{
		return false;
	}
	out.Set(cell.X, cell.Y, cell.Z);
	out *= this->m_CellSize;
	out += this->m_Area->Bounds.Origin - this->m_Area->Bounds.BoxExtent;

	out.X += this->m_CellSize.X / 2.0f;
	out.Y += this->m_CellSize.Y / 2.0f;
	out.Z += this->m_CellSize.Z / 2.0f;

	return true;
}

ABlock* ABuildArea::SpawnBlockAt(const FIntVector& cell, TSubclassOf<ABlock> blockClass, AActor *source) const
{
	FVector location;
	if (!this->GetGridLocation(cell, location))
	{
		return nullptr;
	}
	return ABlock::SpawnBlock(Super::GetWorld(), blockClass, this->m_Team, source, location);
}

void ABuildArea::OnBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
	UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter *character = Cast<APlayerCharacter>(OtherActor);
	if (character != nullptr && character->GetTeam() == this->m_Team)
	{
		character->SetBuildArea(this);
	}
}

#if WITH_EDITOR
void ABuildArea::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(ABuildArea, m_CellSize))
	{
		this->CalculateGridSize();
	}
	if (name == GET_MEMBER_NAME_CHECKED(ABuildArea, m_DebugCellSizeMesh) && this->m_DebugCellSizeMesh != nullptr)
	{
		FVector origin, extent;
		this->m_DebugCellSizeMesh->GetActorBounds(false, origin, extent);
		this->SetCellSize(extent * 2.0f);
		this->m_DebugCellSizeMesh = nullptr;
	}
	if (name == GET_MEMBER_NAME_CHECKED(ABuildArea, m_DebugForceUpdate))
	{
		this->CalculateGridSize();
		this->m_DebugForceUpdate = false;
	}
	if (name == GET_MEMBER_NAME_CHECKED(ABuildArea, m_DebugSnapPlaced))
	{
		FActorIterator itr(Super::GetWorld());
		while (itr)
		{
			FVector origin, extent;
			itr->GetActorBounds(false, origin, extent);

			FIntVector cell;
			if (itr->IsA(ABlock::StaticClass()) && this->GetGridCell(origin + extent / 2.0f, cell))
			{
				FVector out;
				this->GetGridLocation(cell, out);
				itr->SetActorRotation(FRotator(0.0f));
				itr->SetActorLocation(out);
			}
			++itr;
		}
		this->m_DebugSnapPlaced = false;
	}
	if (name == GET_MEMBER_NAME_CHECKED(ABuildArea, m_DebugConstrain))
	{
		FActorIterator itr(Super::GetWorld());
		while (itr)
		{
			FVector origin, extent;
			itr->GetActorBounds(false, origin, extent);

			FIntVector cell;
			if (itr->IsA(ABlock::StaticClass()) && this->GetGridCell(origin + extent / 2.0f, cell))
			{
				FBodyInstance *body = ((ABlock*)*itr)->GetMesh()->GetBodyInstance();
				if (body != nullptr)
				{
					body->SetDOFLock(EDOFMode::Default);
					body->bLockXRotation = true;
					body->bLockYRotation = true;
					body->bLockZRotation = true;
					body->bLockXTranslation = true;
					body->bLockYTranslation = true;
				}
			}
			++itr;
		}
		this->m_DebugConstrain = false;
	}
	if (name == GET_MEMBER_NAME_CHECKED(ABuildArea, m_DebugTeam))
	{
		FVector origin, extent;
		Super::GetActorBounds(false, origin, extent);
		FBox bounds = FBox(origin - extent, origin + extent);

		FActorIterator itr(Super::GetWorld());
		while (itr)
		{
			const FVector& position = itr->GetActorLocation();
			if (itr->IsA(ABlock::StaticClass()) && bounds.IsInside(position))
			{
				Cast<ABlock>(*itr)->SetTeam(this->m_DebugTeam);
			}
			++itr;
		}
		this->m_DebugTeam = 0;
	}
}
#endif