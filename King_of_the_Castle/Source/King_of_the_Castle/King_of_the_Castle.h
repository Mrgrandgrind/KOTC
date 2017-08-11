// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#ifndef __KING_OF_THE_CASTLE_H__
#define __KING_OF_THE_CASTLE_H__

#include "EngineMinimal.h"

#define PI2 (PI * 2.0f)
#define DEG_TO_RAD (PI / 180.0f)
#define RAD_TO_DEG (180.0f / PI)

// Configuration //
#define KOTC_EVENTS_ENABLED true	// Whether or not events should be triggered (does not disable ability to manually trigger events)

#define KOTC_CONSTRUCTION_CHAIN_ENABLED true	// Whether or not chaining (click-and-dragging blocks) should work
#define KOTC_CONSTRUCTION_CHAIN_AXIS_LOCK true	// Whether or not to lock the axis when chaining blocks

#define KOTC_CONSTRUCTION_BLOCK_REACH 1.0f
//#define KOTC_CONSTRUCTION_REACH_DISTANCE 1.8f		// Number of blocks the player can reach using the brush
//#define KOTC_CONSTRUCTION_REACH_MULTIPLIER 150.0f //Multiplier for distance. 150 is the size of a basic block.
//////////////////

// Cheats //
//////////// All cheats should be disabled before commit //
#define KOTC_CONSTRUCTION_INFINITE_BLOCKS false	// Whether or not the player should have infinite blocks
#define KOTC_CONSTRUCTION_QUICK_CRAFT false		// Makes all crafting times equal to 1/4 of a second
////////////

////
#define VECTOR_SPLIT(vector) vector.X, vector.Y, vector.Z
#define VECTOR3_TO_VECTOR2(vector) vector.X, vector.Y

#define UE_LOG_TEXT(text) UE_LOG(LogClass, Log, TEXT(text));
#define UE_LOG_PTR(ptr) UE_LOG(LogClass, Log, TEXT("%p"), ptr)
#define UE_LOG_RAND() UE_LOG(LogClass, Log, TEXT("%d"), FMath::Rand())
#define UE_LOG_INT(value) UE_LOG(LogClass, Log, TEXT("%d"), value)
#define UE_LOG_FLOAT(value) UE_LOG(LogClass, Log, TEXT("%f"), value)
#define UE_LOG_STRING(text) UE_LOG(LogClass, Log, TEXT("%s"), text)
#define UE_LOG_VECTOR(vector) UE_LOG(LogClass, Log, TEXT("%f, %f, %f"), VECTOR_SPLIT(vector))
#define UE_LOG_INTVECTOR(vector) UE_LOG(LogClass, Log, TEXT("%d, %d, %d"), VECTOR_SPLIT(vector))
////

template<typename T>
FORCEINLINE static T* GetGameMode(class UWorld *world)
{
	return Cast<T>(world->GetAuthGameMode());
}

FORCEINLINE static class ABaseGameMode* GetGameMode(class UWorld *world)
{
	return reinterpret_cast<ABaseGameMode*>(world->GetAuthGameMode());
}
#endif
