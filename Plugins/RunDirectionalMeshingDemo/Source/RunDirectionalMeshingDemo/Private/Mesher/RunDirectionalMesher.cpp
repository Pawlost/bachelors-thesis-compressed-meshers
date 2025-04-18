﻿#include "Mesher/RunDirectionalMesher.h"

#include "Mesher/MeshingUtils/MesherVariables.h"
#include "Mesher/MeshingUtils/VoxelChange.h"
#include "Voxel/Grid/VoxelGrid.h"

class URealtimeMeshSimple;

void URunDirectionalMesher::GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange)
{
	if (EmptyActor(MeshVars))
	{
		return;
	}

#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Mesh generation")
#endif

	const auto VoxelGridPtr = Cast<UVoxelGrid>(MeshVars.ChunkParams.OriginalChunk->VoxelModel);
    
	if (VoxelGridPtr == nullptr)
    {
    	return;
    }
	
	auto& VoxelGrid = *VoxelGridPtr;

	if (VoxelChange != nullptr)
	{
		ChangeVoxelId(VoxelGrid, MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable, *VoxelChange);
	}
	
	InitFaceContainers(MeshVars);
	FaceGeneration(VoxelGrid, MeshVars);
	DirectionalGreedyMeshing(MeshVars);
	GenerateMeshFromFaces(MeshVars);
}

void URunDirectionalMesher::FaceGeneration(UVoxelGrid& VoxelGridObject, FMesherVariables& faceParams) const
{
#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Run lenght meshing generation")
#endif

	const auto ChunkDimension = VoxelGenerator->GetVoxelCountPerChunkDimension();
	
	// Traverse through voxel grid
	for (uint32 x = 0; x < ChunkDimension; x++)
	{
		// Border is necessary to know if voxels from neighboring chunk are needed.
		const auto bMinBorder = IsMinBorder(x);
		const auto bMaxBorder = IsMaxBorder(x);

		// Get last voxel coordinate which is different for each face
		const auto XAxisIndex = VoxelGenerator->CalculateVoxelIndex(x, 0, 0);
		const auto YAxisIndex = VoxelGenerator->CalculateVoxelIndex(0, x, 0);
		const auto ZAxisIndex = VoxelGenerator->CalculateVoxelIndex(0, 0, x);

		for (uint32 z = 0; z < ChunkDimension; z++)
		{
			for (uint32 y = 0; y < ChunkDimension; y++)
			{
				/*
				* Increment run for each chunk axis
				* Coordinates are different in order to create a sorted array of quads/faces.
				* Sorted array is then used in Run Direction Greedy meshing, because it is sorted the array needs to traversed only once.
				* Grid is only read from, it doesn't matter which coordinate is first
				* Each voxel needs to be visited only once per face.
				* Indices are same for face and reversed face.
				*/
				IncrementRun(x, y, z, XAxisIndex, bMinBorder, bMaxBorder, FrontFaceTemplate, BackFaceTemplate,
				             faceParams, VoxelGridObject);
				IncrementRun(y, x, z, YAxisIndex, bMinBorder, bMaxBorder, RightFaceTemplate, LeftFaceTemplate,
				             faceParams, VoxelGridObject);
				IncrementRun(z, y, x, ZAxisIndex, bMinBorder, bMaxBorder, BottomFaceTemplate, TopFaceTemplate,
				             faceParams, VoxelGridObject);
			}
		}
	}
}

void URunDirectionalMesher::IncrementRun(const int X, const int Y, const int Z, const int32 AxisVoxelIndex, const bool bIsMinBorder, const bool bIsMaxBorder,
                                    const FMeshingDirections& FaceTemplate,
                                    const FMeshingDirections& ReversedFaceTemplate,
                                    FMesherVariables& MeshVars, const UVoxelGrid& VoxelGridObject) const
{
	// Get voxel at current position of the run.
	const auto Position = FIntVector(X, Y, Z);
	const int32 Index = VoxelGenerator->CalculateVoxelIndex(Position);
	const FVoxel Voxel = VoxelGridObject.VoxelGrid->GetData()[Index];
	
	// If voxel is empty, no mesh should be generated
	if (!Voxel.IsEmptyVoxel())
	{
		// Get correct face containers
		auto OriginalChunk = MeshVars.ChunkParams.OriginalChunk;
		const auto LocalVoxelId = MeshVars.VoxelIdToLocalVoxelMap[Voxel.VoxelId];
		const auto FaceContainerIndex = static_cast<uint8>(FaceTemplate.StaticMeshingData.FaceSide);
		const auto FaceContainerVoxelIndex = static_cast<uint8>(ReversedFaceTemplate.StaticMeshingData.FaceSide);
	
		// Generate face for each direction
		AddFace(VoxelGridObject, FaceTemplate, bIsMinBorder, Index, Position, Voxel, AxisVoxelIndex,
		        MeshVars.Faces[FaceContainerIndex][LocalVoxelId], MeshVars.ChunkParams);
		AddFace(VoxelGridObject, ReversedFaceTemplate, bIsMaxBorder, Index, Position, Voxel, AxisVoxelIndex,
		        MeshVars.Faces[FaceContainerVoxelIndex][LocalVoxelId], MeshVars.ChunkParams);
	}
}

void URunDirectionalMesher::AddFace(const UVoxelGrid& VoxelGridObject, const FMeshingDirections& FaceTemplate, const bool bIsBorder,
                               const int32& Index, const FIntVector& Position, const FVoxel& Voxel,
                               const int32& AxisVoxelIndex,
                               const TSharedPtr<TArray<FChunkFace>>& ChunkFaces, const FChunkParams& ChunkParams)
{
	// Calculate indices need to check if face should be generated
	const FVoxelIndexParams VoxelIndexParams =
	{
		bIsBorder,
		FaceTemplate.ForwardVoxelIndex + Index,
		FaceTemplate.PreviousVoxelIndex + Index,
		Index - AxisVoxelIndex + FaceTemplate.ChunkBorderIndex,
		Voxel,
		FaceTemplate.StaticMeshingData.FaceSide
	};

	// Check if face should be generated
	if (IsBorderVoxelVisible(VoxelIndexParams, ChunkParams) || IsVoxelVisible(VoxelGridObject, VoxelIndexParams))
	{
		// Generate new face with coordinates
		const FChunkFace NewFace = FaceTemplate.StaticMeshingData.FaceCreator(Voxel, Position, 1);

		if (!ChunkFaces->IsEmpty())
		{
			// Tries to merge face coordinates into previous face. Because faces are sorted, the last one is always the correct one.
			FChunkFace& PrevFace = ChunkFaces->Last();
			
			if (FaceTemplate.StaticMeshingData.RunDirectionFaceMerge(PrevFace, NewFace))
			{
				// Return when new face was merged
				return;
			}
		}

		ChunkFaces->Push(NewFace);
	}
}

bool URunDirectionalMesher::IsBorderVoxelVisible(const FVoxelIndexParams& FaceData, const FChunkParams& ChunkStruct)
{
	if (FaceData.IsBorder)
	{
		// Check voxel visibility in side chunk (crosschunk)
		const auto FaceContainerIndex = static_cast<uint8>(FaceData.FaceDirection);
		const auto SideChunk = ChunkStruct.SideChunks[FaceContainerIndex];
		if (SideChunk != nullptr)
		{
			const auto& NextVoxel =  SideChunk->VoxelModel->GetVoxelAtIndex(FaceData.CurrentVoxelIndex);
			return NextVoxel.IsTransparent() && NextVoxel != FaceData.CurrentVoxel;
		}
	
		return SideChunk == nullptr && ChunkStruct.ShowBorders;
	}
	return false;
}

bool URunDirectionalMesher::IsVoxelVisible(const UVoxelGrid& VoxelGridObject, const FVoxelIndexParams& FaceData)
{
	if (!FaceData.IsBorder && VoxelGridObject.VoxelGrid->IsValidIndex(FaceData.ForwardVoxelIndex))
	{
		// Check if next voxel is visible based on calculated index
		const auto NextVoxel = VoxelGridObject.VoxelGrid->GetData()[FaceData.ForwardVoxelIndex];
		return NextVoxel.IsTransparent() && NextVoxel != FaceData.CurrentVoxel;
	}
	return false;
}

void URunDirectionalMesher::DirectionalGreedyMeshing(const FMesherVariables& MeshVars)
{
#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Run directional greedy mesh generation")
#endif

	// Merge faces in sorted arrays
	for (uint8 f = 0; f < CHUNK_FACE_COUNT; f++)
	{
		for (const auto VoxelId : MeshVars.VoxelIdToLocalVoxelMap)
		{
			auto FaceContainer = MeshVars.Faces[f][VoxelId.Value];
			const int LastElementIndex = FaceContainer->Num() - 1;

			// Iterate from last face
			for (int32 i = LastElementIndex - 1; i >= 0; i--)
			{
				FChunkFace& NextFace = (*FaceContainer)[i + 1];

				int BackTrackIndex = i;

				/*
				 * Face may be last in the coordinate row.
				 * It is necessary to iterate through the current coordinate row to reach previous coordinate row.
				 * It is necessary to go only -1 coordinate because if there is a merge, it will accumulate.
				 */
				while (FaceContainer->IsValidIndex(BackTrackIndex))
				{
					FChunkFace& Face = (*FaceContainer)[BackTrackIndex];

					if (Face.StartVertexUp.Z < NextFace.StartVertexDown.Z)
					{
						// Break the iteration if coordinate row is too far.
						break;
					}

					if (FChunkFace::MergeFaceUp(Face, NextFace))
					{
						// Break the iteration if merge was found
						FaceContainer->RemoveAt(i + 1, EAllowShrinking::No);
						break;
					}

					BackTrackIndex--;
				}
			}
		}
	}
}

void URunDirectionalMesher::ChangeVoxelId(const UVoxelGrid& VoxelGridObject, TMap<int32, uint32>& VoxelTable, const FVoxelChange& VoxelChange) const
{
	const auto Index = VoxelGenerator->CalculateVoxelIndex(VoxelChange.VoxelPosition);
	const FVoxel VoxelId = VoxelGenerator->GetVoxelByName(VoxelChange.VoxelName);
	
	// Check if chunk and position is valid.
	if (VoxelGridObject.VoxelGrid->IsValidIndex(Index))
	{
		// Default unknown voxels are empty
		if (VoxelId.IsEmptyVoxel())
		{
			const FVoxel RemovedVoxel = VoxelGridObject.VoxelGrid->GetData()[Index];
			VoxelGenerator->RemoveVoxelFromChunkTable(VoxelTable, RemovedVoxel);

			// Make previous voxel position empty.
			VoxelGridObject.VoxelGrid->GetData()[Index] = VoxelId;
		}
		else
		{
			// If voxel is known we get specific Id
			VoxelGenerator->ChangeKnownVoxelAtIndex(*VoxelGridObject.VoxelGrid, VoxelTable, Index, VoxelId);
		}
	}
}