#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Voxel/Generator/Single/SingleVoxelGeneratorBase.h"
#include "SpecificVoxelGenerator.generated.h"

UCLASS()
class DATASET_API USpecificVoxelGenerator : public USingleVoxelGeneratorBase
{
	GENERATED_BODY()
	
public:
	virtual void GenerateVoxels(FChunk& Chunk) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel")
	TArray<FIntVector> SpecificPositions;
};
