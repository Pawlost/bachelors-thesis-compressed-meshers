#pragma once

#include "VoxelChange.generated.h"

USTRUCT()
struct RDVMRLECOMPRESSED_API FVoxelChange
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FName VoxelName;

	UPROPERTY(EditAnywhere)
	FIntVector VoxelPosition;
};