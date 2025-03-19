#pragma once

#include "CoreMinimal.h"
#include "Voxel/Voxel.h"
#include "RLEVoxel.generated.h"

/**
 * Represents a single voxel in the voxel grid.
 * Contains an ID and a transparency flag.
 */
USTRUCT()
struct RLERUNDIRECTIONALMESHINGDEMO_API FRLEVoxel
{
	GENERATED_BODY()

	// Tied to FVoxel namespace
	static constexpr int32 EMPTY_VOXEL = -1; 
	
	// Saving voxels is not implemented in this demo, but a property specifier for saving has been added.
	UPROPERTY(SaveGame)
	uint32 RunLenght;
	FVoxel voxel;
	
	FORCEINLINE bool IsEmptyVoxel() const
	{
		return voxel.IsEmptyVoxel();
	}

	FORCEINLINE bool IsTransparent() const
	{
		 return voxel.IsTransparent();
	}
};
