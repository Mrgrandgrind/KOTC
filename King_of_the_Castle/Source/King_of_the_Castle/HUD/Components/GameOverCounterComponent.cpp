// Fill out your copyright notice in the Description page of Project Settings.

#include "King_of_the_Castle.h"
#include "GameOverCounterComponent.h"

UGameOverCounterComponent::UGameOverCounterComponent()
{
	this->m_Size = 60.0f;
	this->m_Counter = 0.0f;
	this->m_Duration = 7.0f;
	this->m_FadeInDuration = 0.75f;

	this->m_CountMaterial = nullptr;
	this->m_CountMaterialDynamic = nullptr;

	Super::m_bRender = false;
	Super::m_bRenderOnLast = true;
}

void UGameOverCounterComponent::Render(AGameHUD* hud, const FVector2D& origin, const FVector2D& extent, const float& scale)
{
	this->m_Counter += hud->GetWorld()->GetDeltaSeconds();

	UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(this->m_CountMaterialDynamic);
	if(material == nullptr)
	{
		if(this->m_CountMaterial == nullptr)
		{
			return;
		}
		this->m_CountMaterialDynamic = material = UMaterialInstanceDynamic::Create(this->m_CountMaterial, Super::GetOuter());
		material->SetScalarParameterValue(TEXT("Time Delay"), 0.0f);
		material->SetScalarParameterValue(TEXT("Time Multiplier"), 1.0f / this->m_Duration);
	}
	material->SetScalarParameterValue(TEXT("Time"), FMath::Min(this->m_Counter, this->m_Duration));
	material->SetScalarParameterValue(TEXT("Alpha"), FMath::Sin(HALF_PI * FMath::Min(this->m_Counter / this->m_FadeInDuration, 1.0f)));

	float size = this->m_Size * scale;
	hud->DrawMaterialSimple(this->m_CountMaterialDynamic, origin.X - size / 2.0f, origin.Y - size / 2.0f, size, size);
}
