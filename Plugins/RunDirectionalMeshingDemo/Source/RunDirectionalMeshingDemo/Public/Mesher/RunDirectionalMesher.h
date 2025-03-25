#pragma once
#include "CoreMinimal.h"
#include "MesherBase.h"
#include "Mesh/RealtimeMeshDataStream.h"
#include "MeshingUtils/MeshingDirections.h"
#include "MeshingUtils/VoxelChange.h"
#include "RunDirectionalMesher.generated.h"

class UVoxelGrid;
struct FChunkParams;

UCLASS(ClassGroup=(Meshers), Blueprintable)
class RUNDIRECTIONALMESHINGDEMO_API URunDirectionalMesher : public UMesherBase
{
	GENERATED_BODY()

public:
	virtual void GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange) override;
	
private:
	struct FVoxelIndexParams
	{
		bool IsBorder;
		int32 ForwardVoxelIndex;
		int32 PreviousVoxelIndex;
		int32 CurrentVoxelIndex;
		FVoxel CurrentVoxel;
		EFaceDirection FaceDirection;
	};

	static bool IsBorderVoxelVisible(const FVoxelIndexParams& FaceData, const FChunkParams& ChunkStruct);
	static bool IsVoxelVisible(const UVoxelGrid& VoxelGridObject, const FVoxelIndexParams& FaceData);

	void IncrementRun(int X, int Y, int Z, int32 AxisVoxelIndex, bool bIsMinBorder, bool bIsMaxBorder,
	                  const FMeshingDirections& FaceTemplate, const FMeshingDirections& ReversedFaceTemplate,
	                  FMesherVariables& MeshVars, UVoxelGrid& VoxelGridObject) const;

	static void AddFace(const UVoxelGrid& VoxelGridObject,const FMeshingDirections& FaceTemplate, bool bIsBorder,
	                    const int32& Index, const FIntVector& Position, const FVoxel& Voxel,
	                    const int32& AxisVoxelIndex,
	                    const TSharedPtr<TArray<FChunkFace>>& ChunkFaces, const FChunkParams& ChunkParams);

	void InitFaceContainers(FMesherVariables& MeshVars) const;
	void FaceGeneration(UVoxelGrid& VoxelGridObject, FMesherVariables& MeshVars) const;
	static void DirectionalGreedyMeshing(const FMesherVariables& MeshVars);
	void GenerateMeshFromFaces(const FMesherVariables& MeshVars) const;

	void ChangeVoxelId(UVoxelGrid& VoxelGridObject, TMap<int32, uint32>& VoxelTable, const FVoxelChange& VoxelChange) const;
	
	void GenerateActorMesh(const TMap<uint32, uint16>& LocalVoxelTable,
	                       const RealtimeMesh::FRealtimeMeshStreamSet& StreamSet,
	                       const TSharedPtr<FChunkParams>& ChunkParams) const;
};
