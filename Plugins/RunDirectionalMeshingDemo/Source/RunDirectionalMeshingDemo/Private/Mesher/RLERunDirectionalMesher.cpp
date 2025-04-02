#include "Mesher/RLERunDirectionalMesher.h"
#include "Mesher/RunDirectionalMesher.h"

#include "Mesher/MeshingUtils/MesherVariables.h"
#include "Voxel/RLEVoxel.h"
#include "Voxel/Grid/RLEVoxelGrid.h"

void URLERunDirectionalMesher::CompressVoxelGrid(FChunk& Chunk, TArray<FVoxel>& VoxelGrid)
{
	auto VoxelGridObject = NewObject<URLEVoxelGrid>();
	VoxelGridObject->VoxelGrid.Reserve(Chunk.ChunkVoxelIdTable.Num());

	VoxelGridObject->VoxelGrid.Emplace(1, VoxelGrid[0]);

	for (int32 x = 1; x < VoxelGrid.Num(); x++)
	{
		const FVoxel Voxel = VoxelGrid[x];
		if (VoxelGridObject->VoxelGrid.Last().Voxel == Voxel)
		{
			VoxelGridObject->VoxelGrid.Last().RunLenght++;
		}
		else
		{
			VoxelGridObject->VoxelGrid.Emplace(1, Voxel);
		}
	}

	Chunk.VoxelModel = VoxelGridObject;
}

void URLERunDirectionalMesher::GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange)
{
	if (EmptyActor(MeshVars))
	{
		return;
	}

#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Mesh generation")
#endif

	const auto VoxelGridPtr = Cast<URLEVoxelGrid>(MeshVars.ChunkParams.OriginalChunk->VoxelModel);

	if (VoxelGridPtr == nullptr)
	{
		return;
	}

	auto& VoxelGrid = VoxelGridPtr->VoxelGrid;

	TArray<FRLEVoxel> NewVoxelGrid;
	FVoxel EditVoxel;

	//Measure---
	int voxelCount = 0;
	//---

	if (VoxelChange != nullptr)
	{
		NewVoxelGrid.Reserve(VoxelGrid.Num() + 1);
		EditVoxel = VoxelGenerator->GetVoxelByName(VoxelChange->VoxelName);


		// Measure---
		UE_LOG(LogTemp, Warning, TEXT("Edit Voxel: %d, Position: X %d,  Y %d, Z %d,"), EditVoxel.VoxelId,
		       VoxelChange->VoxelPosition.X, VoxelChange->VoxelPosition.Y, VoxelChange->VoxelPosition.Z);

		for (auto count : MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable)
		{
			voxelCount += count.Value;
		}
		//----

		if (!EditVoxel.IsEmptyVoxel())
		{
			auto& VoxelTable = MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable;
			if (!VoxelTable.Contains(EditVoxel.VoxelId))
			{
				VoxelTable.Add(EditVoxel.VoxelId, 0);
			}
		}
	}

	InitFaceContainers(MeshVars);

	bool bEdited = false;
	int32 RunIndex = -1;
	int32 EditAreaIndex = 0;
	FRLEVoxel CurrentRLEVoxel = VoxelGrid[0];
	auto TraversedRun = CurrentRLEVoxel.RunLenght;
	int YEnd;
	FVoxel ReplacedVoxel = FVoxel();

	const int ChunkDimension = VoxelGenerator->GetVoxelCountPerChunkDimension();

	// Traverse through voxel grid
	for (int x = 0; x < ChunkDimension; x++)
	{
		for (int z = 0; z < ChunkDimension; z++)
		{
			int YStart = 0;
			bool bIsNewRun;

			// Calculate index
			while (YStart < ChunkDimension)
			{
				if (TraversedRun == CurrentRLEVoxel.RunLenght)
				{
					if (EditAreaIndex == 0)
					{
						bIsNewRun = true;
						RunIndex++;
						CurrentRLEVoxel = VoxelGrid[RunIndex];
					}
					else
					{
						bIsNewRun = false;
						CurrentRLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - EditAreaIndex];
						EditAreaIndex--;
					}

					YEnd = CurrentRLEVoxel.RunLenght;
					TraversedRun = 0;
				}
				else
				{
					bIsNewRun = false;
					YEnd = CurrentRLEVoxel.RunLenght - TraversedRun;
				}

				// No action is need when in edit area, otherwise try to apply change and add next run to a new array
				if (VoxelChange != nullptr)
				{
					// Add only full runs
					if (bIsNewRun)
					{
						// Add only outside edit area
						NewVoxelGrid.Push(CurrentRLEVoxel);
					}

					if (!bEdited && x == VoxelChange->VoxelPosition.X && z == VoxelChange->VoxelPosition.Z &&
						YStart <= VoxelChange->VoxelPosition.Y)
					{

						const auto RunEnd = YStart + YEnd;
						if (RunEnd >= VoxelChange->VoxelPosition.Y)
						{
							//MID RUN EDIT
							bEdited = true;

							if (!CalculateEditIndexMidRun(TraversedRun, NewVoxelGrid, CurrentRLEVoxel, VoxelChange, YStart,
															 RunEnd, EditVoxel, EditAreaIndex, VoxelGrid, RunIndex, ReplacedVoxel))
							{
								return;
							}

							continue;
						}
					}
				}

				//MEASURE--------
				if (NewVoxelGrid.Num() > 1 && NewVoxelGrid.Last().Voxel == NewVoxelGrid[NewVoxelGrid.Num() - 2].Voxel)
				{
					UE_LOG(LogTemp, Error, TEXT("HERE"));
				}
				//-----

				// Step to end
				if (YStart + YEnd > ChunkDimension)
				{
					YEnd = ChunkDimension - YStart;
				}

				if (!CurrentRLEVoxel.IsVoxelEmpty())
				{
					// Generate run faces
					auto InitialPosition = FIntVector(x, YStart, z);

					// Front
					CreateFace(MeshVars, FStaticMergeData::FrontFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Back
					CreateFace(MeshVars, FStaticMergeData::BackFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Top
					CreateFace(MeshVars, FStaticMergeData::TopFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Bottom
					CreateFace(MeshVars, FStaticMergeData::BottomFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Right
					CreateFace(MeshVars, FStaticMergeData::RightFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Left
					CreateFace(MeshVars, FStaticMergeData::LeftFaceData, InitialPosition, CurrentRLEVoxel, YEnd);
				}

				TraversedRun += YEnd;
				YStart += YEnd;
			}

			if (VoxelChange != nullptr && !bEdited && VoxelChange->VoxelPosition.Y == 0 && (x == VoxelChange->VoxelPosition.X && z + 1 ==
				VoxelChange->VoxelPosition.Z || x + 1 == VoxelChange->VoxelPosition.X && VoxelChange->VoxelPosition.Z == 0 && z == 31))
			{
				bEdited = true;
				
				if (!CalculateEditIndexEndRun(NewVoxelGrid, TraversedRun, EditVoxel,
												 CurrentRLEVoxel,
												 ReplacedVoxel, EditAreaIndex, VoxelGrid, RunIndex))
				{
					return;
				}
			}
		}
	}

	GenerateMeshFromFaces(MeshVars);

	if (VoxelChange != nullptr)
	{
		int VoxelDimension = VoxelGenerator->GetVoxelCountPerChunk();
		int TestRunIndex = 0;

		//MEASURE ----
		for (int i = NewVoxelGrid.Num() - 1; i > 0; i--)
		{
			TestRunIndex += NewVoxelGrid[i].RunLenght;

			if (NewVoxelGrid[i].RunLenght <= 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Wrong run: %d"), NewVoxelGrid[i].RunLenght);
			}

			if (NewVoxelGrid[i].Voxel == NewVoxelGrid[i - 1].Voxel)
			{
				auto firstRun = NewVoxelGrid[i];
				auto secondRun = NewVoxelGrid[i - 1];
				UE_LOG(LogTemp, Error, TEXT("Grid Index1: %d, Grid Index2: %d"), i, i-1);
				UE_LOG(LogTemp, Error, TEXT("Voxel: %d"), firstRun.Voxel.VoxelId);
				UE_LOG(LogTemp, Error, TEXT("First Run: %d"), firstRun.RunLenght);
				UE_LOG(LogTemp, Error, TEXT("Second Run: %d"), secondRun.RunLenght);
				UE_LOG(LogTemp, Error, TEXT("Reversed Run Index: %d"), TestRunIndex);
			}
		}
		//-----------

		TestRunIndex += NewVoxelGrid[0].RunLenght;

		//MEASURE ----
		if (VoxelDimension != TestRunIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("Run lenght does not match: %d; Dimension %d"), TestRunIndex, VoxelDimension);
		}
		//-----

		if (!EditVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[EditVoxel.VoxelId]++;
		}
		else
		{
			//MEASURE ----
			voxelCount--;
		}

		if (!ReplacedVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[ReplacedVoxel.VoxelId]--;

			if (MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[ReplacedVoxel.VoxelId] <= 0)
			{
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Remove(ReplacedVoxel.VoxelId);
			}
		}
		else
		{
			//MEASURE ----
			voxelCount++;
		}

		//MEASURE ----
		int Count = 0;
		for (auto count : MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable)
		{
			Count += count.Value;
		}

		if (Count != voxelCount)
		{
			UE_LOG(LogTemp, Error, TEXT("Voxel count is incorrect, original count: %d; Count %d"), voxelCount, Count);
		}
		//------

		// TODO: remove this
		if (NewVoxelGrid.Num() == 1 && NewVoxelGrid[0].IsVoxelEmpty())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
		}
		else if (NewVoxelGrid.Num() >= 1 && NewVoxelGrid.Num() <= 3)
		{
			bool empty = true;
			for (auto Run : NewVoxelGrid)
			{
				if (!Run.IsVoxelEmpty())
				{
					empty = false;
				}
			}

			if (empty)
			{
				NewVoxelGrid[0].RunLenght += NewVoxelGrid.Last().RunLenght;
				NewVoxelGrid.RemoveAt(1);
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
			}
		}

		VoxelGridPtr->VoxelGrid = NewVoxelGrid;
	}
}

void URLERunDirectionalMesher::CreateFace(FMesherVariables& MeshVars, const FStaticMergeData& StaticData,
                                          const FIntVector& InitialPosition, const FRLEVoxel& RLEVoxel, const int YEnd)
{
	const int LocalVoxelId = MeshVars.VoxelIdToLocalVoxelMap[RLEVoxel.Voxel.VoxelId];
	const FChunkFace NewFace = StaticData.FaceCreator(RLEVoxel.Voxel, InitialPosition, YEnd);
	const auto FaceContainerIndex = static_cast<uint8>(StaticData.FaceSide);
	MeshVars.Faces[FaceContainerIndex][LocalVoxelId]->Push(NewFace);
}

bool URLERunDirectionalMesher::CalculateEditIndexMidRun(
	const int TraversedRun, TArray<FRLEVoxel>& NewVoxelGrid, FRLEVoxel& CurrentRLERun, const FVoxelChange* VoxelChange, const int YStart,
	const int RunEnd, const FVoxel& EditVoxel, int& EditAreaIndex, TArray<FRLEVoxel>& VoxelGrid, int& RunIndex,
	FVoxel& ReplacedVoxel)
{
	if (RunEnd == VoxelChange->VoxelPosition.Y)
	{
		auto& LastRLERun = NewVoxelGrid.Last();
		
		if (LastRLERun.Voxel == EditVoxel)
		{
			LastRLERun.RunLenght++;
		}
		else
		{
			NewVoxelGrid.Emplace(1, EditVoxel);
			EditAreaIndex = 1;
		}

		if (VoxelGrid.IsValidIndex(RunIndex + 1))
		{
			auto& NextRLERun = VoxelGrid[RunIndex + 1];

			if (NextRLERun.Voxel == EditVoxel)
			{
				return false;
			}

			NextRLERun.RunLenght--;
			ReplacedVoxel = NextRLERun.Voxel;

			if (NextRLERun.IsRunEmpty())
			{
				RunIndex++;

				if (VoxelGrid.IsValidIndex(RunIndex + 1))
				{
					NextRLERun = VoxelGrid[RunIndex + 1];
					if (NextRLERun.Voxel == LastRLERun.Voxel)
					{
						RunIndex++;
						LastRLERun.RunLenght += NextRLERun.RunLenght;
					}
				}
			}
		}

		CurrentRLERun = LastRLERun;
	}
	else if (CurrentRLERun.Voxel != EditVoxel)
	{
		auto& LastRLERun = NewVoxelGrid.Last();
		const auto MidRunLenght = TraversedRun + VoxelChange->VoxelPosition.Y - YStart;

		const FRLEVoxel SplitRLERun(LastRLERun.RunLenght - MidRunLenght - 1, LastRLERun.Voxel);

		LastRLERun.RunLenght = MidRunLenght;
		ReplacedVoxel = LastRLERun.Voxel;
		CurrentRLERun = LastRLERun;

		if (SplitRLERun.IsRunEmpty())
		{
			const int NextIndex = RunIndex + 1;
			if (VoxelGrid.IsValidIndex(NextIndex) && VoxelGrid[NextIndex].Voxel == EditVoxel)
			{
				VoxelGrid[NextIndex].RunLenght++;
			}else
			{
				NewVoxelGrid.Emplace(1, EditVoxel);
				EditAreaIndex = 1;
			}
		}
		else
		{
			NewVoxelGrid.Emplace(1, EditVoxel);
			NewVoxelGrid.Push(SplitRLERun);
			EditAreaIndex = 2;
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool URLERunDirectionalMesher::CalculateEditIndexEndRun(
	TArray<FRLEVoxel>& NewVoxelGrid, int& TraversedRun, FVoxel& EditVoxel,
	FRLEVoxel& RLEVoxel,
	FVoxel& ReplacedVoxel, int& EditAreaIndex,
	TArray<FRLEVoxel>& VoxelGrid, int& RunIndex)
{
	auto& LastRun = NewVoxelGrid.Last();

	if (TraversedRun != LastRun.RunLenght)
	{

		if (LastRun.Voxel == EditVoxel)
		{
			return false;
		}

		auto& lastRLE = NewVoxelGrid.Last();
		int runLenght = lastRLE.RunLenght - TraversedRun - 1;
		lastRLE.RunLenght = TraversedRun;
		NewVoxelGrid.Emplace(1, EditVoxel);
		NewVoxelGrid.Emplace(runLenght, RLEVoxel.Voxel);
		TraversedRun = 0;
		RLEVoxel.RunLenght = 0;

		ReplacedVoxel = RLEVoxel.Voxel;

		auto& lastRLE2 = NewVoxelGrid.Last();

		if (lastRLE2.RunLenght <= 0)
		{
			NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
			EditAreaIndex = 1;
			if (VoxelGrid.IsValidIndex(RunIndex + 1))
			{
				auto& nextnextRLEVoxel = VoxelGrid[RunIndex + 1];

				if (nextnextRLEVoxel.Voxel == EditVoxel)
				{
					nextnextRLEVoxel.RunLenght++;
					NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
					EditAreaIndex = 0;
				}
			}
		}
		else
		{
			EditAreaIndex = 2;
		}

		return true;
	}
	else
	{
		if (LastRun.Voxel == EditVoxel)
		{
			TraversedRun = LastRun.RunLenght;
			EditAreaIndex = 0;
			LastRun.RunLenght += 1;

			ReplacedVoxel = LastRun.Voxel;

			if (VoxelGrid.IsValidIndex(RunIndex + 1))
			{
				auto& nextnextRLEVoxel = VoxelGrid[RunIndex + 1];
				nextnextRLEVoxel.RunLenght--;
				if (nextnextRLEVoxel.RunLenght <= 0)
				{
					RunIndex++;
					if (VoxelGrid.IsValidIndex(RunIndex + 1))
					{
						auto& nextnextnextRLEVoxel = VoxelGrid[RunIndex + 1];

						if (nextnextnextRLEVoxel.Voxel == LastRun.Voxel)
						{
							RunIndex++;
							LastRun.RunLenght += nextnextnextRLEVoxel.RunLenght;
						}
					}
				}
			}

			RLEVoxel.RunLenght = LastRun.RunLenght;
			return true;
		}

		if (VoxelGrid.IsValidIndex(RunIndex + 1))
		{
			
			auto& nextRLEVoxel = VoxelGrid[RunIndex + 1];

			if (nextRLEVoxel.Voxel == EditVoxel)
			{
				return false;
			}

			NewVoxelGrid.Emplace(1, EditVoxel);
			nextRLEVoxel.RunLenght--;
			EditAreaIndex = 1;

			ReplacedVoxel = nextRLEVoxel.Voxel;

			if (nextRLEVoxel.RunLenght <= 0)
			{
				RunIndex++;
				if (VoxelGrid.IsValidIndex(RunIndex + 1))
				{
					auto& nextnextRLEVoxel = VoxelGrid[RunIndex + 1];

					if (nextnextRLEVoxel.Voxel == LastRun.Voxel && nextnextRLEVoxel.Voxel == EditVoxel)
					{
						RunIndex++;
						LastRun.RunLenght += nextnextRLEVoxel.RunLenght;
					}
					else if (nextnextRLEVoxel.Voxel == EditVoxel)
					{
						NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
						EditAreaIndex = 0;
						nextnextRLEVoxel.RunLenght += 1;
					}
				}
			}
		}
	}

	return true;
}
