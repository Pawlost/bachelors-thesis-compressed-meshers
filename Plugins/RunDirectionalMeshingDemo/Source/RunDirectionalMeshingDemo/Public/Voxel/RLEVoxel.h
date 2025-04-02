#pragma once

#include "CoreMinimal.h"
#include "Voxel/Voxel.h"
#include "RLEVoxel.generated.h"

/**
 * Represents a single voxel in the voxel grid.
 * Contains an ID and a transparency flag.
 */
USTRUCT()
struct RUNDIRECTIONALMESHINGDEMO_API FRLEVoxel
{
	GENERATED_BODY()

	// Tied to FVoxel namespace
	static constexpr int32 EMPTY_VOXEL = -1; 
	
	// Saving voxels is not implemented in this demo, but a property specifier for saving has been added.
	UPROPERTY(SaveGame)
	int RunLenght;
	FVoxel Voxel;
	
	FORCEINLINE bool IsVoxelEmpty() const
	{
		return Voxel.IsEmptyVoxel();
	}

	FORCEINLINE bool IsTransparent() const
	{
		return Voxel.IsTransparent();
	}

	FORCEINLINE bool IsRunEmpty() const
	{
		return RunLenght <= 0;
	}
};
