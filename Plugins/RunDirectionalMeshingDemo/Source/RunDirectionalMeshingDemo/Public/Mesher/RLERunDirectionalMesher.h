#pragma once
#include "CoreMinimal.h"
#include "Mesher/MesherBase.h"
#include "Voxel/RLEVoxel.h"
#include "RLERunDirectionalMesher.generated.h"

struct FChunkParams;

UCLASS(ClassGroup=(Meshers), Blueprintable)
class RUNDIRECTIONALMESHINGDEMO_API URLERunDirectionalMesher : public UMesherBase
{
	GENERATED_BODY()

public:
	virtual void GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange) override;
	virtual void CompressVoxelGrid(FChunk& Chunk, TArray<FVoxel>& VoxelGrid) override;

private:
	struct FVoxelIndexParams
	{
		bool IsBorder;
		int32 ForwardVoxelIndex;
		int32 PreviousVoxelIndex;
		int32 CurrentVoxelIndex;
		FRLEVoxel CurrentVoxel;
		EFaceDirection FaceDirection;
	};

	struct FIndexParams
	{
		TSharedPtr<TArray<FRLEVoxel>> NewVoxelGrid;
		TSharedPtr<TArray<FRLEVoxel>> VoxelGrid;
		
		int TraversedRun;
		int YStart;
		FVoxel EditVoxel;
		FRLEVoxel RLEVoxel;
		FVoxel ReplacedVoxel;
		int32 EditAreaIndex;
		int32 RunIndex;
		
		FRLEVoxel CurrentRLERun;
		FVoxelChange* VoxelChange = nullptr; 
	};
	
	static void CreateFace(FMesherVariables& MeshVars, const FStaticMergeData& StaticData, const FIntVector& InitialPosition, const FRLEVoxel& RLEVoxel, const int YEnd);

	//TODO: Types
	static bool CalculateMidRunEditIndex(FIndexParams& IndexParams, int RunEnd);

	static bool CalculateEndRunEditIndex(FIndexParams& IndexParams);

	static void CalculateSplitRun(const FRLEVoxel& SplitRLERun, FIndexParams& IndexParams);
};
