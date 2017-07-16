#include "King_of_the_Castle.h"
#include "BlockBrush.h"

#include "Gamemode/BaseGameMode.h"
#include "Character/PlayerCharacter.h"
#include "Construction/BlockData.h"
#include "Construction/BuildArea.h"
#include "Construction/BlockEntity.h"
#include "Construction/BlockStructureManager.h"

#include "Runtime/Engine/Classes/Engine/TextRenderActor.h"
#include "Runtime/Engine/Classes/Components/BillboardComponent.h"
#include "Runtime/Engine/Classes/Components/TextRenderComponent.h"

#define BRUSH_MESH_LOCATION TEXT("StaticMesh'/Game/Meshes/M_BuildBrush.M_BuildBrush'")
#define BRUSH_MATERIAL_LOCATION TEXT("Material'/Game/Materials/M_Brush.M_Brush'")

#define DOOR_BLOCK_LOCATION TEXT("/Game/Blueprints/Construction/BP_DoorBlock")
#define GOLDEN_BLOCK_LOCATION TEXT("/Game/Blueprints/Construction/BP_GoldenBlock")
#define CONSTRUCTION_BLOCK_LOCATION TEXT("/Game/Blueprints/Construction/BP_ConstructionBlock")

#define DEFAULT_MAX_BLOCK_COUNT 10

#define BRUSH_POSITION_OFFSET FVector(0.0f, Super::Bounds.BoxExtent.Y, 0.0f)

#define BRUSH_COLOR_NAME TEXT("Color")
#define BRUSH_UNDEFINED_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
#define BRUSH_CREATE_COLOR FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)
#define BRUSH_DESTROY_COLOR FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)

#define BLOCK_CONSTRUCTION_COLOR FColor(55, 77, 55)
#define BLOCK_DOOR_COLOR FColor(122, 228, 231)
#define BLOCK_GOLD_COLOR FColor(173, 122, 73)

#define BRUSH_CREATE_TOP_PERCENTAGE 0.25f // The top of the block is defined by this percentage. (i.e. we pretend the top 25% of the block surface is the top)

UBlockBrush::UBlockBrush() : m_Team(nullptr), m_TextActor(nullptr), m_Material(nullptr)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(BRUSH_MESH_LOCATION);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(BRUSH_MATERIAL_LOCATION);

	if (Mesh.Succeeded())
	{
		Super::SetStaticMesh(Mesh.Object);
	}
	this->m_Material = Material.Object;

	Super::SetCollisionProfileName(TEXT("OverlapAll"));
	Super::SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	Super::bOnlyOwnerSee = true;
	Super::SetCastShadow(false);

	this->m_bDebugRenderTrace = true;
}

void UBlockBrush::BeginPlay()
{
	Super::BeginPlay();

	if (Super::GetWorld() != nullptr)
	{
		this->m_TextActor = Super::GetWorld()->SpawnActor<ATextRenderActor>();
#if WITH_EDITOR
		this->m_TextActor->SetActorLabel(TEXT("BlockCountText"));
		this->m_TextActor->GetSpriteComponent()->bVisible = false;
#endif

		UTextRenderComponent *text = this->m_TextActor->GetTextRender();
		text->SetText(NSLOCTEXT("", "", ""));
		text->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
		text->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
		text->XScale = 4.0f;
		text->YScale = 4.0f;
		text->bOnlyOwnerSee = true;
		text->SetTextRenderColor(FColor::Silver);

		this->m_TextActor->SetOwner(Super::GetAttachmentRootActor());
		this->m_TextActor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	this->UBlockBrush::SetBrushVisible(false);
}

void UBlockBrush::SetChainMode(const bool& enable)
{
	this->m_bChainMode = enable;
	this->m_Chained.Empty();

	if (enable)
	{
		this->m_Chained.Add(this->m_ActiveCell);
	}
}

UMaterialInstanceDynamic* UBlockBrush::GetMaterialDynamic()
{
	UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(Super::GetMaterial(0));
	if (material == nullptr)
	{
		material = UMaterialInstanceDynamic::Create(this->m_Material, this->GetOuter());
		Super::SetMaterial(0, material);
	}
	return material;
}

bool UBlockBrush::IsSupport(const FVector& position, const FVector& cellSize) const
{
	ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
	if (gamemode != nullptr && gamemode->GetStructureManager() != nullptr)
	{
		return gamemode->GetStructureManager()->IsSupport(position, cellSize / 2.0f);
	}
	return true;
}

bool UBlockBrush::IsOverlapped() const
{
	TArray<UPrimitiveComponent*> components;
	this->GetOverlappingComponents(components);
	for (int i = 0; i < components.Num(); i++)
	{
		// If we are overlapping with a players capsule component
		ACharacter *character = Cast<ACharacter>(components[i]->GetAttachmentRootActor());
		if (character != nullptr && components[i] == character->GetCapsuleComponent())
		{
			return true;
		}
		AActor *actor = components[i]->GetAttachmentRootActor();
		if (actor != nullptr && actor->IsA(ABlockEntity::StaticClass()))
		{
			continue;
		}
		// If we are overlapping with a staticmesh component
		if (components[i]->IsA(UStaticMeshComponent::StaticClass())
			&& !components[i]->IsA(UBlockBrush::StaticClass()))
		{
			return true;
		}
	}
	return false;
}

void UBlockBrush::UpdateCountText(UBlockData *data, const FVector *ownerLocation)
{
	if (this->m_TextActor == nullptr)
	{
		return;
	}
	UTextRenderComponent *render = this->m_TextActor->GetTextRender();
	if (data != nullptr)
	{
		render->SetText(FText::Format(NSLOCTEXT("", "", "{0}"), FText::AsNumber(data->GetCount())));
	}
	if (ownerLocation != nullptr)
	{
		FRotator rotation = (*ownerLocation - Super::GetComponentLocation()).Rotation();
		rotation.Yaw = int(FMath::GridSnap(rotation.Yaw, 90.0f)) % 360;
		render->SetRelativeRotation(FRotator(0.0f, rotation.Yaw, 0.0f));

		const float offset = 6; //text position offset
		const FVector& extent = Super::Bounds.BoxExtent;
		switch (int(rotation.Yaw))
		{
			case 0:
				this->m_TextActor->SetActorRelativeLocation(FVector(extent.X - offset, -extent.Y, 0.0f));
				break;
			case 90:
			case -270:
				this->m_TextActor->SetActorRelativeLocation(FVector(0.0f, -offset, 0.0f));
				break;
			case -90:
			case 270:
				this->m_TextActor->SetActorRelativeLocation(FVector(0.0f, -extent.Y * 2.0f + offset, 0.0f));
				break;
			case 180:
			case -180:
				this->m_TextActor->SetActorRelativeLocation(FVector(-extent.X + offset, -extent.Y, 0.0f));
				break;
			default: break;
		}
	}
}

void UBlockBrush::Update(APlayerCharacter *character, ABuildArea *area, const FHitResult& trace)
{
	this->m_LastTrace = trace;

	if (trace.GetComponent() == nullptr)
	{
		this->RenderTrace(trace.TraceStart, trace.TraceEnd, FColor::Orange);
	}
	else
	{
		this->RenderTrace(trace.TraceStart, trace.ImpactPoint, FColor::Green);
		this->RenderTrace(trace.ImpactPoint, trace.TraceEnd, FColor::Orange);
	}

	bool show = false;
	bool pre = this->OnPreCheck(area, trace, this->m_ActiveCell, show),
		main = this->OnMainCheck(area, trace, this->m_ActiveCell, show, pre),
		post = this->OnPostCheck(area, trace, this->m_ActiveCell, show, pre, main);

	this->m_bPositionValid = main && post; // We only need the main and post checks to pass

	if (this->m_bPositionValid)
	{
		FVector location;
		this->m_bPositionValid = area->GetGridLocation(this->m_ActiveCell, location);

		Super::SetWorldLocation(location + BRUSH_POSITION_OFFSET);
	}

	show = show && this->m_bPositionValid; // If show is true, only do it if the position is valid
	if (this->IsBrushVisible() != show)
	{
		this->SetBrushVisible(show);
	}

	//FVector location = character->GetActorLocation();
	//this->UpdateCountText(nullptr, &location);
}
