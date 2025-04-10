// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FastNoise/VoxelFastNoise.h"
#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelGeneratorWrapper.generated.h"

UCLASS(Blueprintable)
class UVoxelGeneratorWrapper : public UVoxelGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float NoiseHeight = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	int32 Seed = 1337;

	//~ Begin UVoxelGenerator Interface
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	//~ End UVoxelGenerator Interface
};

class FVoxelGeneratorExampleInstance : public TVoxelGeneratorInstanceHelper<FVoxelGeneratorExampleInstance, UVoxelGeneratorWrapper>
{
public:
    using Super = TVoxelGeneratorInstanceHelper<FVoxelGeneratorExampleInstance, UVoxelGeneratorWrapper>;

	explicit FVoxelGeneratorExampleInstance(UVoxelGeneratorWrapper& MyGenerator);

	//~ Begin FVoxelGeneratorInstance Interface
	virtual void Init(const FVoxelGeneratorInit& InitStruct) override;

	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;

	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;

	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final;
	//~ End FVoxelGeneratorInstance Interface

private:
	const float NoiseHeight;
	const int32 Seed;
	FVoxelFastNoise Noise;
};