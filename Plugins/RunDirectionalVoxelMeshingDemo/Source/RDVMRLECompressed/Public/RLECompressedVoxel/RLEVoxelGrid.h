#pragma once

#include "CoreMinimal.h"
#include "VoxelModel/VoxelModel.h"
#include "RLEVoxelGrid.generated.h"

UCLASS()
class RDVMRLECOMPRESSED_API URLEVoxelGrid : public UVoxelModel
{
	GENERATED_BODY()

public:
	TSharedPtr<TArray<FRLEVoxel>> RLEVoxelGrid;
};
