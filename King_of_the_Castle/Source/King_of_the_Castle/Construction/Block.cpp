// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"

#include "BlockEntity.h"
#include "BlockStructureManager.h"
#include "Gamemode/BaseGameMode.h"

#include "Block.h"

#define MATERIAL_DAMAGE_NAME TEXT("DamageColor")
#define BLOCK_DEFAULT_MAX_HEALTH_COLOR FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
#define BLOCK_DEFAULT_MIN_HEALTH_COLOR FLinearColor(0.9f, 0.1f, 0.1f, 1.0f)

// Sets default values
ABlock::ABlock() : m_PointValue(1), m_Health(BLOCK_DEFAULT_HEALTH), 
m_MaxHealth(BLOCK_DEFAULT_HEALTH), m_Team(-1)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(BLOCK_DEFAULT_MESH_LOCATION);

	this->m_Mesh = UObject::CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMeshComponent"));
	this->m_Mesh->SetSimulatePhysics(false);
	if (Mesh.Succeeded())
	{
		this->m_Mesh->SetStaticMesh(Mesh.Object);
	}
	Super::RootComponent = this->m_Mesh;

	this->m_BlockMinHealthColor = BLOCK_DEFAULT_MIN_HEALTH_COLOR;
	this->m_BlockMaxHealthColor = BLOCK_DEFAULT_MAX_HEALTH_COLOR;
	Super::SetActorScale3D(FVector(BLOCK_DEFAULT_SCALE));

	//this->GetMesh()->GetBodyInstance()->AngularDamping = 100000000.0f;
	//
	//this->GetMesh()->GetBodyInstance()->SetMaxAngularVelocity(0.0f, false);
	//this->GetMesh()->GetBodyInstance()->AddCustomPhysics(FCalculateCustomPhysics())
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
	
	this->m_Mesh->SetMassOverrideInKg(NAME_None, BLOCK_DEFAULT_MASS);

	FBodyInstance *body = this->GetMesh()->GetBodyInstance();
	if (body != nullptr)
	{
		body->bLockXRotation = true;
		body->bLockYRotation = true;
		body->bLockZRotation = true;
		body->bLockXTranslation = true;
		body->bLockYTranslation = true;
		body->bLockZTranslation = false;
		body->SetDOFLock(EDOFMode::SixDOF);
	}
}

// Drop block
TArray<ABlockEntity*> ABlock::DropBlock(AActor *source, const bool& restrictPickup)
{
	TArray<ABlockEntity*> entites = ABlockEntity::SpawnBlockEntity(this, source, restrictPickup);
	this->DestroyBlock();
	return entites;
}

// Destroy block
void ABlock::DestroyBlock(AActor *source)
{
	ABaseGameMode *gamemode = Cast<ABaseGameMode>(Super::GetWorld()->GetAuthGameMode());
	if (gamemode != nullptr)
	{
		gamemode->OnBlockDestroy(this, source);
	}
	Super::Destroy();
}

// Get material in dynamic form
UMaterialInstanceDynamic* ABlock::GetDynamicMaterial() const
{
	UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_Mesh->GetMaterial(0));
	if (material == nullptr)
	{
		material = UMaterialInstanceDynamic::Create(this->m_Mesh->GetMaterial(0), Super::RootComponent);
		this->m_Mesh->SetMaterial(0, material);
	}
	return material;
}

// Static spawn block method
ABlock* ABlock::SpawnBlock(UWorld *world, TSubclassOf<ABlock> type, const int& team, 
	AActor *source, const FVector& location, const FRotator& rotation)
{
	if (world == nullptr)
	{
		return nullptr;
	}
	ABlock *block = world->SpawnActor<ABlock>(type, location, rotation);
	if (block != nullptr)
	{
		block->SetTeam(team);

		ABaseGameMode *gamemode = Cast<ABaseGameMode>(world->GetAuthGameMode());
		if (gamemode != nullptr)
		{
			gamemode->OnBlockPlace(block, source);
		}
	}
	return block;
}

// Change health of block
void ABlock::SetHealth(const float& health)
{
	this->m_Health = FMath::Max(health, 0.0f);

	UMaterialInstanceDynamic *material = this->GetDynamicMaterial();
	if (material == nullptr)
	{
		return;
	}
	float healthPerc = FMath::Clamp(this->m_Health / this->m_MaxHealth, 0.0f, 1.0f);
	if (this->m_Health == 0.0f)
	{
		healthPerc = 1.0f;
	}
	FLinearColor color = this->m_BlockMinHealthColor + (this->m_BlockMaxHealthColor - this->m_BlockMinHealthColor) * healthPerc;
	material->SetVectorParameterValue(MATERIAL_DAMAGE_NAME, color);
}

float ABlock::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if(!this->IsDestructable())
	{
		return 0.0f;
	}
	float actual = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	this->SetHealth(this->GetHealth() - actual);
	if (this->m_Health <= 0.0f) // if dead
	{
		this->DropBlock(DamageCauser, true);
	}
	return actual;
}

#if WITH_EDITOR
void ABlock::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName name = event.MemberProperty != nullptr ? event.MemberProperty->GetFName() : NAME_None;

	if (name == GET_MEMBER_NAME_CHECKED(ABlock, m_bDebugBreakBlock))
	{
		this->DropBlock(nullptr, false);
		this->m_bDebugBreakBlock = false;
	}
}
#endif