#pragma once
#include "CoreMinimal.h"
#include "Mesh/RealtimeMeshDataStream.h"
#include "Mesher/MesherBase.h"
#include "Voxel/RLEVoxel.h"
#include "RLERunDirectionalMesher.generated.h"

struct FChunkParams;

UCLASS(ClassGroup=(Meshers), Blueprintable)
class RUNDIRECTIONALMESHINGDEMO_API URLERunDirectionalMesher : public UMesherBase
{
	GENERATED_BODY()

public:
	virtual void GenerateMesh(FMesherVariables& MeshVars) override;
	virtual void CompressVoxelGrid(FMesherVariables& MeshVars, TArray<FVoxel>& VoxelGrid);

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

	static bool IsBorderVoxelVisible(const FVoxelIndexParams& FaceData, const FChunkParams& ChunkStruct);
	static bool IsVoxelVisible(const FVoxelIndexParams& FaceData, const FChunkParams& ChunkStruct);

	void IncrementRun(int X, int Y, int Z, int32 AxisVoxelIndex, bool bIsMinBorder, bool bIsMaxBorder,
	                  const FMeshingDirections& FaceTemplate, const FMeshingDirections& ReversedFaceTemplate,
	                  FMesherVariables& MeshVars) const;

	static void AddFace(const FMeshingDirections& FaceTemplate, bool bIsBorder,
	                    const int32& Index, const FIntVector& Position, const FVoxel& Voxel,
	                    const int32& AxisVoxelIndex,
	                    const TSharedPtr<TArray<FChunkFace>>& ChunkFaces, const FChunkParams& ChunkParams);

	void InitFaceContainers(FMesherVariables& MeshVars) const;
	void FaceGeneration(FMesherVariables& MeshVars) const;
	static void DirectionalGreedyMeshing(const FMesherVariables& MeshVars);
	void GenerateMeshFromFaces(const FMesherVariables& MeshVars) const;
	
	void GenerateActorMesh(const TMap<uint32, uint16>& LocalVoxelTable,
	                       const RealtimeMesh::FRealtimeMeshStreamSet& StreamSet,
	                       const TSharedPtr<FChunkParams>& ChunkParams) const;
};
