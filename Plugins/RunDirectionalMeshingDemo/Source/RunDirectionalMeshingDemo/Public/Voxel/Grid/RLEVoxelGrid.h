#pragma once

#include "CoreMinimal.h"
#include "VoxelData.h"
#include "Voxel/RLEVoxel.h"
#include "RLEVoxelGrid.generated.h"

UCLASS()
class RUNDIRECTIONALMESHINGDEMO_API URLEVoxelGrid : public UVoxelData
{
	GENERATED_BODY()

public:
	TArray<FRLEVoxel> VoxelGrid;
};
