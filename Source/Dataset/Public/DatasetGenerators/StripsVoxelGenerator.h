#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Voxel/Generator/Single/SingleVoxelGeneratorBase.h"
#include "StripsVoxelGenerator.generated.h"

UCLASS()
class DATASET_API UStripsVoxelGenerator : public USingleVoxelGeneratorBase
{
	GENERATED_BODY()
	
public:
	virtual void GenerateVoxels(FChunk& Chunk) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0"), Category = "Voxel")
	int32 XGap = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0"), Category = "Voxel")
	int32 YGap = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0"), Category = "Voxel")
	int32 ZGap = 1;
private:
	static bool IsInGap(const int Coordinate, const int StripDimension);
};
