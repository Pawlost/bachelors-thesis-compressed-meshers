#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VoxelData.generated.h"

UCLASS()
class RUNDIRECTIONALMESHINGDEMO_API UVoxelData : public UObject
{
	GENERATED_BODY()

	public:
		virtual FVoxel GetVoxelAtIndex(int32 Index) PURE_VIRTUAL( UVoxelData::GetVoxelAtIndex, return FVoxel(); );
};
