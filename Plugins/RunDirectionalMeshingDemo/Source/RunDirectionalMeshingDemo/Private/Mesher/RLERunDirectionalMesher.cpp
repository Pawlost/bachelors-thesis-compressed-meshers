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
	int32 GlobalIndex = -1;
	int32 EditAreaIndex = 0;
	auto RLEVoxel = VoxelGrid[0];
	auto CurrentRunLenght = RLEVoxel.RunLenght;
	auto YEnd = 0;
	FVoxel ReplacedVoxel = FVoxel();

	const int ChunkDimension = VoxelGenerator->GetVoxelCountPerChunkDimension();

	// Traverse through voxel grid
	for (int x = 0; x < ChunkDimension; x++)
	{
		for (int z = 0; z < ChunkDimension; z++)
		{
			int YStart = 0;
			bool bPrev = false;

			// Calculate index
			while (YStart < ChunkDimension)
			{
				if (CurrentRunLenght == RLEVoxel.RunLenght)
				{
					if (EditAreaIndex == 0)
					{
						bPrev = false;
						GlobalIndex++;
						RLEVoxel = VoxelGrid[GlobalIndex];
					}
					else
					{
						bPrev = true;
						RLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - EditAreaIndex];
						EditAreaIndex--;
					}

					YEnd = RLEVoxel.RunLenght;
					CurrentRunLenght = 0;
				}
				else
				{
					YEnd = RLEVoxel.RunLenght - CurrentRunLenght;
				}

				//Edit area
				if (VoxelChange != nullptr && !bPrev)
				{
					if (CurrentRunLenght == 0)
					{
						//Add only full runs
						NewVoxelGrid.Add(RLEVoxel);
					}

					if (bEdited == false && x == VoxelChange->VoxelPosition.X && z == VoxelChange->VoxelPosition.Z &&
						YStart <= VoxelChange->VoxelPosition.Y && YStart + YEnd >= VoxelChange->
						VoxelPosition.Y)
					{
						//EDITED HERE
						bEdited = true;
						bool end = false;
						
						if (CalculateEditIndexMidRun(CurrentRunLenght, NewVoxelGrid, RLEVoxel, VoxelChange, YStart, YEnd, EditVoxel, EditAreaIndex, VoxelGrid, GlobalIndex, ReplacedVoxel, end))
						{
							if (end)
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
					UE_LOG(LogTemp, Warning, TEXT("HERE"));
				}
				//-----

				// Step to end
				if (YStart + YEnd > ChunkDimension)
				{
					YEnd = ChunkDimension - YStart;
				}

				if (!RLEVoxel.IsEmptyVoxel())
				{
					// Generate run faces
					auto InitialPosition = FIntVector(x, YStart, z);

					// Front
					CreateFace(MeshVars, FStaticMergeData::FrontFaceData, InitialPosition, RLEVoxel, YEnd);

					// Back
					CreateFace(MeshVars, FStaticMergeData::BackFaceData, InitialPosition, RLEVoxel, YEnd);

					// Top
					CreateFace(MeshVars, FStaticMergeData::TopFaceData, InitialPosition, RLEVoxel, YEnd);
				
					// Bottom
					CreateFace(MeshVars, FStaticMergeData::BottomFaceData, InitialPosition, RLEVoxel, YEnd);

					// Right
					CreateFace(MeshVars, FStaticMergeData::RightFaceData, InitialPosition, RLEVoxel, YEnd);

					// Left
					CreateFace(MeshVars, FStaticMergeData::LeftFaceData, InitialPosition, RLEVoxel, YEnd);
				}
				
				CurrentRunLenght += YEnd;
				YStart += YEnd;
			}
			
			CalculateEditIndexEndRun(VoxelChange, bEdited, x, z, NewVoxelGrid, CurrentRunLenght, EditVoxel, RLEVoxel, ReplacedVoxel, EditAreaIndex, VoxelGrid, GlobalIndex);
		}
	}
	
	GenerateMeshFromFaces(MeshVars);

	if (VoxelChange != nullptr)
	{
		int VoxelDimension = VoxelGenerator->GetVoxelCountPerChunk();
		int RunIndex = 0;
		
		//MEASURE ----
		for (int i = NewVoxelGrid.Num() - 1; i > 0; i--)
		{
			RunIndex += NewVoxelGrid[i].RunLenght;

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
				UE_LOG(LogTemp, Error, TEXT("Reversed Run Index: %d"), RunIndex);
			}
		}
		//-----------

		RunIndex += NewVoxelGrid[0].RunLenght;

		//MEASURE ----
		if (VoxelDimension != RunIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("Run lenght does not match: %d; Dimension %d"), RunIndex, VoxelDimension);
		}
		//-----
		
		if (!EditVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[EditVoxel.VoxelId]++;
		}else
		{
			//MEASURE ----
			voxelCount--;
		}
		
		if (!ReplacedVoxel.IsEmptyVoxel()){
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[ReplacedVoxel.VoxelId]--;
			
			if (MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[ReplacedVoxel.VoxelId] <= 0)
			{
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Remove(ReplacedVoxel.VoxelId);
			}
		}else
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
		if (NewVoxelGrid.Num() == 1 && NewVoxelGrid[0].IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
		}
		else if (NewVoxelGrid.Num() >= 1 && NewVoxelGrid.Num() <= 3)
		{
			bool empty = true;
			for (auto Run : NewVoxelGrid)
			{
				if (!Run.IsEmptyVoxel())
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

void URLERunDirectionalMesher::CreateFace(FMesherVariables& MeshVars, const FStaticMergeData& StaticData, const FIntVector& InitialPosition, const FRLEVoxel& RLEVoxel, const int YEnd)
{
	const int LocalVoxelId = MeshVars.VoxelIdToLocalVoxelMap[RLEVoxel.Voxel.VoxelId];
	const FChunkFace NewFace = StaticData.FaceCreator(RLEVoxel.Voxel, InitialPosition, YEnd);
	const auto FaceContainerIndex = static_cast<uint8>(StaticData.FaceSide);
	MeshVars.Faces[FaceContainerIndex][LocalVoxelId]->Push(NewFace);
}

bool URLERunDirectionalMesher::CalculateEditIndexMidRun(int& size, TArray<FRLEVoxel>& NewVoxelGrid, FRLEVoxel& RLEVoxel, const FVoxelChange* VoxelChange, int& startIndex,
	int& lenght, const FVoxel& EditVoxel, int& editIndex, TArray<FRLEVoxel>& VoxelGrid, int& globalIndex, FVoxel& ReplacedVoxel, bool& end)
{
		if (startIndex + lenght == VoxelChange->VoxelPosition.Y)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE1"))
			auto& lastRLE = NewVoxelGrid.Last();
			if (RLEVoxel.Voxel == EditVoxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE2"))
				lastRLE.RunLenght++;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE3"))
				FRLEVoxel NewRLE(1, EditVoxel);
				NewVoxelGrid.Add(NewRLE);
				editIndex = 1;
			}

			if (VoxelGrid.IsValidIndex(globalIndex + 1))
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE4"))
				auto& nextRLEVoxel = VoxelGrid[globalIndex + 1];
				if (nextRLEVoxel.Voxel == EditVoxel)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE5"))
					end = true;
					return true;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE5"))
					nextRLEVoxel.RunLenght--;
					ReplacedVoxel = nextRLEVoxel.Voxel;

					if (nextRLEVoxel.RunLenght <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE6"))
						globalIndex++;

						if (VoxelGrid.IsValidIndex(globalIndex + 1))
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE7"))
							nextRLEVoxel = VoxelGrid[globalIndex + 1];
							if (nextRLEVoxel.Voxel == NewVoxelGrid.Last().Voxel)
							{
								UE_LOG(LogTemp, Warning, TEXT("HERE8"))
								globalIndex++;
								NewVoxelGrid.Last().RunLenght += nextRLEVoxel.RunLenght;
							}
						}
					}
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("HERE9"))
			//size + lenght
			RLEVoxel = lastRLE;
			lenght = RLEVoxel.RunLenght - size;
		}
		else if (RLEVoxel.Voxel != EditVoxel)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE10"))
			auto& lastRLE = NewVoxelGrid.Last();
			auto newRunLenght = size + VoxelChange->VoxelPosition.Y - startIndex;

			FRLEVoxel NewRLE(1, EditVoxel);
			FRLEVoxel nextRLE(RLEVoxel.RunLenght - newRunLenght - 1, RLEVoxel.Voxel);

			UE_LOG(LogTemp, Warning, TEXT("HERE14"))
			lastRLE.RunLenght = newRunLenght;
							
			ReplacedVoxel = lastRLE.Voxel;

			RLEVoxel = lastRLE;

			if (lastRLE.RunLenght <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE15"))
				NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
			}

			if (!NewVoxelGrid.IsEmpty() && NewVoxelGrid.Last().Voxel == NewRLE.Voxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE16"))
				NewRLE.RunLenght--;
			}

			if (nextRLE.RunLenght > 0 && NewRLE.RunLenght > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE17"))
				if (NewRLE.Voxel == nextRLE.Voxel)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE18"))
					nextRLE.RunLenght++;
					editIndex = 1;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE19"))
					if (NewRLE.RunLenght > 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE20"))
						NewVoxelGrid.Add(NewRLE);
						editIndex = 2;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE21"))
						NewVoxelGrid.Last().RunLenght++;
						startIndex++;
						editIndex = 1;
					}
				}
				NewVoxelGrid.Add(nextRLE);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE22"))
				bool add = true;
				if (VoxelGrid.IsValidIndex(globalIndex + 1))
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE23"))
					auto& nextRLEVoxel = VoxelGrid[globalIndex + 1];
					if (nextRLEVoxel.Voxel == NewRLE.Voxel)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE24"))
						if (NewRLE.RunLenght <= 0)
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE25"))
							globalIndex++;
							size = NewVoxelGrid.Last().RunLenght;
							NewVoxelGrid.Last().RunLenght += nextRLEVoxel.RunLenght + 1;
							RLEVoxel = NewVoxelGrid.Last();
							return true;
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE26"))
							nextRLEVoxel.RunLenght++;
						}
						add = false;
					}
					else if (NewRLE.RunLenght <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE27"))
						NewVoxelGrid.Last().RunLenght++;
						startIndex++;
						return true;
					}
				}

				if (add)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE28"))
					NewVoxelGrid.Add(NewRLE);
					editIndex = 1;
				}
			}


			if ((RLEVoxel.RunLenght <= 0 || newRunLenght == 0) && editIndex > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE29"))
				RLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - editIndex];
				editIndex--;
			}

			lenght = RLEVoxel.RunLenght - size;
		}else
		{
			end = true;
			return true;
		}

	if (lenght == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HERE30"))
		return true;
	}

	return false;
}

void URLERunDirectionalMesher::CalculateEditIndexEndRun(const FVoxelChange* VoxelChange, bool& edited, int x, int z, TArray<FRLEVoxel>& NewVoxelGrid, int& size, FVoxel& EditVoxel, FRLEVoxel& RLEVoxel,
                              FVoxel& ReplacedVoxel, int& editIndex, TArray<FRLEVoxel>& VoxelGrid, int& globalIndex)
{
	if (VoxelChange != nullptr && edited == false && x == VoxelChange->VoxelPosition.X && z + 1 == VoxelChange->
	VoxelPosition.Z &&
	VoxelChange->VoxelPosition.Y == 0)
	{
		edited = true;

		auto& lastVoxel = NewVoxelGrid.Last();

		if (size != lastVoxel.RunLenght)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE31"))

			if (lastVoxel.Voxel == EditVoxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE32"))
				return;
			}

			auto& lastRLE = NewVoxelGrid.Last();
			int runLenght = lastRLE.RunLenght - size - 1;
			lastRLE.RunLenght = size;
			NewVoxelGrid.Emplace(1, EditVoxel);
			NewVoxelGrid.Emplace(runLenght, RLEVoxel.Voxel);
			size = 0;
			RLEVoxel.RunLenght = 0;

			ReplacedVoxel = RLEVoxel.Voxel;

			auto& lastRLE2 = NewVoxelGrid.Last();

			if (lastRLE2.RunLenght <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE33"))
				NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
				editIndex = 1;
				if (VoxelGrid.IsValidIndex(globalIndex + 1))
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE34"))
					auto& nextnextRLEVoxel = VoxelGrid[globalIndex + 1];

					if (nextnextRLEVoxel.Voxel == EditVoxel)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE35"))
						nextnextRLEVoxel.RunLenght++;
						NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
						editIndex = 0;
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE36"))
				editIndex = 2;
			}

			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE55"))
					
			if (lastVoxel.Voxel == EditVoxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE37"))
				size = lastVoxel.RunLenght;
				editIndex = 0;
				lastVoxel.RunLenght += 1;
						
				ReplacedVoxel = lastVoxel.Voxel;

				if (VoxelGrid.IsValidIndex(globalIndex + 1))
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE44"))
					auto& nextnextRLEVoxel = VoxelGrid[globalIndex + 1];
					nextnextRLEVoxel.RunLenght--;
					if (nextnextRLEVoxel.RunLenght <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE45"))
						globalIndex++;
						if (VoxelGrid.IsValidIndex(globalIndex + 1))
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE46"))
							auto& nextnextnextRLEVoxel = VoxelGrid[globalIndex + 1];

							if (nextnextnextRLEVoxel.Voxel == lastVoxel.Voxel)
							{
								UE_LOG(LogTemp, Warning, TEXT("HERE47"))
								globalIndex++;
								lastVoxel.RunLenght += nextnextnextRLEVoxel.RunLenght;
							}
						}
					}
				}

				RLEVoxel.RunLenght = lastVoxel.RunLenght;
				return;
			}

			if (VoxelGrid.IsValidIndex(globalIndex + 1))
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE39"))
				auto& nextRLEVoxel = VoxelGrid[globalIndex + 1];

				if (nextRLEVoxel.Voxel == EditVoxel)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE40"))
					return;
				}

				NewVoxelGrid.Emplace(1, EditVoxel);
				nextRLEVoxel.RunLenght--;
				editIndex = 1;

				ReplacedVoxel = nextRLEVoxel.Voxel;

				if (nextRLEVoxel.RunLenght <= 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE41"))
					globalIndex++;
					if (VoxelGrid.IsValidIndex(globalIndex + 1))
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE42"))
						auto& nextnextRLEVoxel = VoxelGrid[globalIndex + 1];

						if (nextnextRLEVoxel.Voxel == lastVoxel.Voxel)
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE43"))
							globalIndex++;
							lastVoxel.RunLenght += nextnextRLEVoxel.RunLenght;
						}else if (nextnextRLEVoxel.Voxel == EditVoxel)
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE50"))
							NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
							editIndex = 0;
							nextnextRLEVoxel.RunLenght += 1;
						}
					}
				}
			}
		}
	}
}
