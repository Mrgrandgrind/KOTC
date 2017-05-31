// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#ifndef __KING_OF_THE_CASTLE_H__
#define __KING_OF_THE_CASTLE_H__

#include "EngineMinimal.h"

// Configuration //
#define KOTC_EVENTS_ENABLED true	// Whether or not events should be triggered (does not disable ability to manually trigger events)

#define KOTC_CONSTRUCTION_CHAIN_ENABLED true	// Whether or not chaining (click-and-dragging blocks) should work
#define KOTC_CONSTRUCTION_CHAIN_AXIS_LOCK true	// Whether or not to lock the axis when chaining blocks

#define KOTC_CONSTRUCTION_REACH_DISTANCE 4		// Number of blocks the player can reach using the brush
//////////////////

// Cheats //
//////////// All cheats should be disabled before commit //
#define KOTC_CONSTRUCTION_INFINITE_REACH false	// Whether or not the brush reach of the player should be unrestricted
#define KOTC_CONSTRUCTION_INFINITE_BLOCKS false	// Whether or not the player should have infinite blocks
#define KOTC_CONSTRUCTION_QUICK_CRAFT false		// Makes all crafting times equal to 1/4 of a second
////////////

////
#define VECTOR_SPLIT(vector) vector.X, vector.Y, vector.Z
#define UE_LOG_RAND() UE_LOG(LogClass, Log, TEXT("%d"), FMath::Rand())
#define UE_LOG_INT(value) UE_LOG(LogClass, Log, TEXT("%d"), value)
#define UE_LOG_FLOAT(value) UE_LOG(LogClass, Log, TEXT("%f"), value)
#define UE_LOG_STRING(text) UE_LOG(LogClass, Log, TEXT("%s"), text)
#define UE_LOG_VECTOR(vector) UE_LOG(LogClass, Log, TEXT("%f, %f, %f"), VECTOR_SPLIT(vector))
#define UE_LOG_INTVECTOR(vector) UE_LOG(LogClass, Log, TEXT("%d, %d, %d"), VECTOR_SPLIT(vector))
////

#endif
