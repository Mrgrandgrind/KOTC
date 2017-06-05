#include "King_of_the_Castle.h"
#include "BlockData.h"

#include "Block.h"
#include "Brush/PrimaryBrush.h"

#define DEFAULT_BLOCK_COUNT 0
#define DEFAULT_MAX_BLOCK_COUNT 10

UBlockData::UBlockData() : m_Count(DEFAULT_BLOCK_COUNT), m_MaxCount(DEFAULT_MAX_BLOCK_COUNT), m_Material(nullptr)
{
}

int UBlockData::GetCount() const
{
#if KOTC_CONSTRUCTION_INFINITE_BLOCKS
	return 1;
#endif
	return this->m_Count;
}

FName UBlockData::GetNameId() const
{
	ABlock *block = Cast<ABlock>(this->m_ClassType->GetDefaultObject());
	if (block != nullptr)
	{
		return block->GetNameId();
	}
	return FName("Undefined");
}

void UBlockData::SetCount(UPrimaryBrush *brush, const int& count)
{
	this->m_Count = count;
	if (brush == nullptr)
	{
		return;
	}
	UBlockData *data = brush->GetBlockData(brush->GetSelectedIndex());
	//if (data != nullptr && (data == this || data == this->m_Join))
	if (data == this)
	{
		brush->UpdateCountText(data);
	}
}