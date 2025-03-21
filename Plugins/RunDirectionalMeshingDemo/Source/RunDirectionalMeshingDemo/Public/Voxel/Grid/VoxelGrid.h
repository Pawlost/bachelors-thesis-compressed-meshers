#pragma once

#include "CoreMinimal.h"
#include "VoxelData.h"
#include "UObject/Object.h"
#include "Voxel/Voxel.h"
#include "VoxelGrid.generated.h"

UCLASS()
class RUNDIRECTIONALMESHINGDEMO_API UVoxelGrid : public UVoxelData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	TArray<FVoxel> VoxelGrid;
};
