#include "Mesher/RLERunDirectionalMesher.h"
#include "Mesher/RunDirectionalMesher.h"

#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesher/MeshingUtils/MesherVariables.h"
#include "Spawner/ChunkSpawnerBase.h"
#include "Voxel/RLEVoxel.h"
#include "Voxel/Grid/RLEVoxelGrid.h"

void URLERunDirectionalMesher::GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange)
{
	MeshVars.ChunkParams.OriginalChunk->bHasMesh = false;

	if (MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.IsEmpty())
	{
		if (MeshVars.ChunkParams.OriginalChunk->ChunkMeshActor.IsValid())
		{
			// If chunk is full of empty voxels but actor was pulled from pool, clear its mesh
			MeshVars.ChunkParams.OriginalChunk->ChunkMeshActor->ClearMesh();
		}
		return;
	}

#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Mesh generation")
#endif

	auto VoxelGridPtr = Cast<URLEVoxelGrid>(MeshVars.ChunkParams.OriginalChunk->VoxelGrid);

	if (VoxelGridPtr == nullptr)
	{
		return;
	}

	auto& VoxelGrid = VoxelGridPtr->VoxelGrid;

	TArray<FRLEVoxel> NewVoxelGrid;
	FVoxel EditVoxel;
	if (VoxelChange != nullptr)
	{
		NewVoxelGrid.Reserve(VoxelGrid.Num() + 1);
		EditVoxel = VoxelGenerator->GetVoxelByName(VoxelChange->VoxelName);

		if (!EditVoxel.IsEmptyVoxel() && MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Contains(EditVoxel.VoxelId))
		{
			 MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[EditVoxel.VoxelId]++;	
		}else
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Add(EditVoxel.VoxelId, 1);	
		}
	}

	InitFaceContainers(MeshVars);

	bool edited = false;
	int32 globalIndex = -1;
	int32 editIndex = 0;
	auto RLEVoxel = VoxelGrid[0];
	auto size = RLEVoxel.RunLenght;
	auto lenght = 0;

	const int ChunkDimension = VoxelGenerator->GetVoxelCountPerChunkDimension();

	// Traverse through voxel grid
	for (int x = 0; x < ChunkDimension; x++)
	{
		for (int z = 0; z < ChunkDimension; z++)
		{
			int startIndex = 0;
			bool prev = false;
			
			while (startIndex < ChunkDimension)
			{
				if (size == RLEVoxel.RunLenght)
				{
					if (editIndex == 0)
					{
						prev = false;
						globalIndex += 1;
						RLEVoxel = VoxelGrid[globalIndex];
					}
					else
					{
						prev = true;
						RLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - editIndex];
						editIndex--;
					}

					lenght = RLEVoxel.RunLenght;
					size = 0;
				}
				else
				{
					lenght = RLEVoxel.RunLenght - size;
				}

				if (VoxelChange != nullptr && !prev)
				{
				
					if (size == 0)
					{
						//Add only full runs
						NewVoxelGrid.Add(RLEVoxel);
					}

					if (edited == false && x == VoxelChange->VoxelPosition.X && z == VoxelChange->VoxelPosition.Z &&
						startIndex <= VoxelChange->VoxelPosition.Y && startIndex + lenght >= VoxelChange->
						VoxelPosition.Y)
					{
						if (RLEVoxel.Voxel == EditVoxel)
						{
							return;
						}

						//EDITED HERE
						edited = true;

						if (startIndex + lenght == VoxelChange->VoxelPosition.Y)
						{
							auto& lastRLE = NewVoxelGrid.Last();
							if (RLEVoxel.Voxel == EditVoxel)
							{
								lastRLE.RunLenght++;
							}else
							{
								FRLEVoxel NewRLE(1, EditVoxel);
								NewVoxelGrid.Add(NewRLE);
								editIndex = 1;
							}
							
							if (VoxelGrid.IsValidIndex(globalIndex + 1))
							{
								auto& nextRLEVoxel = VoxelGrid[globalIndex + 1];
								if (nextRLEVoxel.Voxel == EditVoxel)
								{
									nextRLEVoxel.RunLenght++;
									editIndex = 0;
									NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
								}else
								{
									nextRLEVoxel.RunLenght--;
									if (nextRLEVoxel.Voxel.VoxelId >= 0)
									{
										MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[nextRLEVoxel.Voxel.VoxelId]--;
									}
								
									if (nextRLEVoxel.RunLenght <= 0)
									{
										globalIndex++;

										if (VoxelGrid.IsValidIndex(globalIndex + 1))
										{
											nextRLEVoxel = VoxelGrid[globalIndex + 1];
											if (nextRLEVoxel.Voxel == NewVoxelGrid.Last().Voxel)
											{
												globalIndex ++;
												NewVoxelGrid.Last().RunLenght += nextRLEVoxel.RunLenght;
											}
										}
									}
								}
							}

							//size + lenght
							RLEVoxel = lastRLE;
							lenght = RLEVoxel.RunLenght - size;
						}
						else
						{
							auto& lastRLE = NewVoxelGrid.Last();
							auto newRunLenght = size + VoxelChange->VoxelPosition.Y - startIndex;
							lastRLE.RunLenght = newRunLenght;
							
							FRLEVoxel NewRLE(1, EditVoxel);
							FRLEVoxel nextRLE(RLEVoxel.RunLenght - lastRLE.RunLenght - 1, RLEVoxel.Voxel);
				
							if (nextRLE.Voxel.VoxelId >= 0)
							{
								MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[nextRLE.Voxel.VoxelId]--;
							}
							
							RLEVoxel = lastRLE;

							if (lastRLE.RunLenght <= 0)
							{
								NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
							}

							if (!NewVoxelGrid.IsEmpty() && NewVoxelGrid.Last().Voxel == NewRLE.Voxel)
							{
								NewRLE.RunLenght--;
							}
							
							if (nextRLE.RunLenght > 0 && NewRLE.RunLenght > 0)
							{
								if (NewRLE.Voxel == nextRLE.Voxel)
								{
									nextRLE.RunLenght++;
									editIndex = 1;
								}else
								{
									if (NewRLE.RunLenght > 0)
									{
										NewVoxelGrid.Add(NewRLE);	
										editIndex = 2;
									}else
									{
										NewVoxelGrid.Last().RunLenght++;
										startIndex++;
										editIndex = 1;
									}
								}
								NewVoxelGrid.Add(nextRLE);
							}else{
								bool add = true;
								if (VoxelGrid.IsValidIndex(globalIndex + 1))
								{
									auto& nextRLEVoxel = VoxelGrid[globalIndex + 1];
									if (nextRLEVoxel.Voxel == NewRLE.Voxel)
									{
										if (NewRLE.RunLenght <= 0)
										{
											globalIndex++;
											size = NewVoxelGrid.Last().RunLenght;
											NewVoxelGrid.Last().RunLenght+=nextRLEVoxel.RunLenght + 1;
											RLEVoxel = NewVoxelGrid.Last();
											continue;
										}else
										{
											nextRLEVoxel.RunLenght++;
										}
										add = false;
									}else if (NewRLE.RunLenght <= 0)
									{
										NewVoxelGrid.Last().RunLenght++;
										startIndex++;
										continue;
									}
								}

								if (add)
								{
									NewVoxelGrid.Add(NewRLE);
									editIndex = 1;
								}
							}

							
							if (RLEVoxel.RunLenght <= 0 && editIndex > 0){
								RLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - editIndex];
								editIndex--;
							}

							lenght = RLEVoxel.RunLenght - size;
							if (lenght == 0)
							{
								continue;
							}
						}
					}
				}

				if (NewVoxelGrid.Num() > 1 && NewVoxelGrid.Last().Voxel == NewVoxelGrid[NewVoxelGrid.Num()-2].Voxel)
				{
					UE_LOG(LogTemp, Warning , TEXT("HERE"));
				}

				if (startIndex + lenght > ChunkDimension)
				{
					lenght = ChunkDimension - startIndex;
				}

				if (RLEVoxel.IsEmptyVoxel())
				{
					size += lenght;
					startIndex += lenght;
					continue;
				}

				auto initialPosition = FIntVector(x, startIndex, z);

				// Generate new face with coordinates


				auto localVoxelId = MeshVars.VoxelIdToLocalVoxelMap[RLEVoxel.Voxel.VoxelId];

				// Front
				FChunkFace NewFace = FChunkFace::CreateFrontFace(RLEVoxel.Voxel, initialPosition, lenght);
				auto FaceContainerIndex = static_cast<uint8>(FFaceToDirection::FrontDirection.FaceSide);
				MeshVars.Faces[FaceContainerIndex][localVoxelId]->Push(NewFace);

				// Back
				NewFace = FChunkFace::CreateBackFace(RLEVoxel.Voxel, initialPosition, lenght);
				FaceContainerIndex = static_cast<uint8>(FFaceToDirection::BackDirection.FaceSide);
				MeshVars.Faces[FaceContainerIndex][localVoxelId]->Push(NewFace);

				// Top
				NewFace = FChunkFace::CreateTopFace(RLEVoxel.Voxel, initialPosition, lenght);
				FaceContainerIndex = static_cast<uint8>(FFaceToDirection::TopDirection.FaceSide);
				MeshVars.Faces[FaceContainerIndex][localVoxelId]->Push(NewFace);

				// Bottom
				NewFace = FChunkFace::CreateBottomFace(RLEVoxel.Voxel, initialPosition, lenght);
				FaceContainerIndex = static_cast<uint8>(FFaceToDirection::BackDirection.FaceSide);
				MeshVars.Faces[FaceContainerIndex][localVoxelId]->Push(NewFace);

				// Right
				NewFace = FChunkFace::CreateRightFace(RLEVoxel.Voxel, initialPosition, lenght);
				FaceContainerIndex = static_cast<uint8>(FFaceToDirection::RightDirection.FaceSide);
				MeshVars.Faces[FaceContainerIndex][localVoxelId]->Push(NewFace);

				// Left
				NewFace = FChunkFace::CreateLeftFace(RLEVoxel.Voxel, initialPosition, lenght);
				FaceContainerIndex = static_cast<uint8>(FFaceToDirection::LeftDirection.FaceSide);
				MeshVars.Faces[FaceContainerIndex][localVoxelId]->Push(NewFace);

				size += lenght;
				startIndex += lenght;
			}
		}
	}
	GenerateMeshFromFaces(MeshVars);

	if (VoxelChange != nullptr)
	{
		int RunIndex = 0;
		for (int i = NewVoxelGrid.Num() - 1; i > 0 ; i--)
		{
			RunIndex += NewVoxelGrid[i].RunLenght;
			if (NewVoxelGrid[i].Voxel == NewVoxelGrid[i-1].Voxel)
			{
				auto firstRun = NewVoxelGrid[i];
				auto secondRun = NewVoxelGrid[i-1];
				UE_LOG(LogTemp, Warning , TEXT("Edit Voxel: %d, Position: X %d,  Y %d, Z %d,"), EditVoxel.VoxelId, VoxelChange->VoxelPosition.X,  VoxelChange->VoxelPosition.Y,  VoxelChange->VoxelPosition.Z);
				UE_LOG(LogTemp, Warning , TEXT("Grid Index1: %d, Grid Index2: %d"), i, i-1);
				UE_LOG(LogTemp, Warning , TEXT("Voxel: %d"), firstRun.Voxel.VoxelId);
				UE_LOG(LogTemp, Warning , TEXT("First Run: %d"), firstRun.RunLenght);
				UE_LOG(LogTemp, Warning , TEXT("Second Run: %d"), secondRun.RunLenght);
				UE_LOG(LogTemp, Warning , TEXT("Reversed Run Index: %d"), RunIndex);
			}
		}

		// TODO: remove this
		if (NewVoxelGrid.Num() == 1 && NewVoxelGrid[0].IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
		}else if (NewVoxelGrid.Num() >= 1 && NewVoxelGrid.Num() <= 3){

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

	// VoxelGrid;
	Chunk.VoxelGrid = VoxelGridObject;
}

void URLERunDirectionalMesher::InitFaceContainers(FMesherVariables& MeshVars) const
{
#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Mesh generation intialization")
#endif

	MeshVars.VoxelIdToLocalVoxelMap.Reserve(MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Num());
	MeshVars.VoxelIdToLocalVoxelMap.Empty();

	for (const auto VoxelId : MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable)
	{
		const auto LocalVoxelId = MeshVars.VoxelIdToLocalVoxelMap.Num();
		MeshVars.VoxelIdToLocalVoxelMap.Add(VoxelId.Key, LocalVoxelId);
	}

	for (uint8 f = 0; f < CHUNK_FACE_COUNT; f++)
	{
		// Voxel faces need to be sorted to different arrays by Id because Realtime Mesh Component requires it
		for (const auto Voxel : MeshVars.VoxelIdToLocalVoxelMap)
		{
			TMap<int32, uint32>& VoxelTable = MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable;
			MeshVars.Faces[f].SetNum(VoxelTable.Num());

			auto FaceArray = MeshVars.Faces[f][Voxel.Value];
			if (FaceArray == nullptr || !FaceArray.IsValid())
			{
				FaceArray = MakeShared<TArray<FChunkFace>>();
				MeshVars.Faces[f][Voxel.Value] = FaceArray;
			}
			else
			{
				// If array was pulled from a pool, just empty it 
				FaceArray->Empty();
			}

			// Preallocate memory needed for meshing
			const uint32 Count = VoxelGenerator->GetVoxelCountPerChunk();
			FaceArray->Reserve(Count);
		}
	}
}

void URLERunDirectionalMesher::GenerateMeshFromFaces(const FMesherVariables& MeshVars) const
{
#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Mesh stream generation")
#endif

	auto StreamSet = MakeShared<FRealtimeMeshStreamSet>();

	TRealtimeMeshBuilderLocal<int32> Builder(*StreamSet.ToWeakPtr().Pin());

	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnableTangents();
	Builder.EnablePolyGroups();

	if (!IsValid(VoxelGenerator))
	{
		return;
	}

	auto VoxelSize = VoxelGenerator->GetVoxelSize();

	// Local voxel table 
	TMap<uint32, uint16> LocalVoxelTable;

	// Iterate through merged faces
	for (auto VoxelId : MeshVars.VoxelIdToLocalVoxelMap)
	{
		for (uint8 FaceIndex = 0; FaceIndex < CHUNK_FACE_COUNT; FaceIndex++)
		{
			auto FaceContainer = MeshVars.Faces[FaceIndex];

			auto SideFaces = FaceContainer[VoxelId.Value];

			auto [Normal, Tangent] = FaceNormalsAndTangents[FaceIndex];

			// Create quad foreach face
			for (auto Face : *SideFaces)
			{
				int32 V0 = Builder.AddVertex(Face.GetFinalStartVertexDown(VoxelSize))
				                  .SetColor(FColor::White)
				                  .SetTexCoord(FVector2f(0, 0))
				                  .SetNormalAndTangent(Normal, Tangent);

				int32 V1 = Builder.AddVertex(Face.GetFinalEndVertexDown(VoxelSize))
				                  .SetColor(FColor::White)
				                  .SetTexCoord(FVector2f(1, 0))
				                  .SetNormalAndTangent(Normal, Tangent);


				int32 V2 = Builder.AddVertex(Face.GetFinalEndVertexUp(VoxelSize))
				                  .SetColor(FColor::White)
				                  .SetTexCoord(FVector2f(1, 1))
				                  .SetNormalAndTangent(Normal, Tangent);

				int32 V3 = Builder.AddVertex(Face.GetFinalStartVertexUp(VoxelSize))
				                  .SetColor(FColor::White)
				                  .SetTexCoord(FVector2f(0, 1))
				                  .SetNormalAndTangent(Normal, Tangent);

				if (!LocalVoxelTable.Contains(VoxelId.Key))
				{
					// Keep track of how many voxel quads are actually displayed
					LocalVoxelTable.Add(VoxelId.Key, LocalVoxelTable.Num());
				}

				// Create quad from 2 triangles
				Builder.AddTriangle(V0, V1, V2, LocalVoxelTable[VoxelId.Key]);
				Builder.AddTriangle(V2, V3, V0, LocalVoxelTable[VoxelId.Key]);
			}
		}
	}

	if (!MeshVars.ChunkParams.OriginalChunk.IsValid() || LocalVoxelTable.IsEmpty())
	{
		return;
	}

	auto Spawner = MakeShared<FChunkParams>(MeshVars.ChunkParams);

	if (!MeshVars.ChunkParams.ExecutedOnMainThread)
	{
		// Synchronize Mesh generation with game thread.
		AsyncTask(ENamedThreads::GameThread, [this, LocalVoxelTable, StreamSet, Spawner]()
		{
			GenerateActorMesh(LocalVoxelTable, *StreamSet, Spawner);
		});
	}
	else
	{
		//Creating AsyncTask from main thread will cause deadlock
		GenerateActorMesh(LocalVoxelTable, *StreamSet, Spawner);
	}

	MeshVars.ChunkParams.OriginalChunk->bHasMesh = true;
}

void URLERunDirectionalMesher::GenerateActorMesh(const TMap<uint32, uint16>& LocalVoxelTable,
                                                 const FRealtimeMeshStreamSet& StreamSet,
                                                 const TSharedPtr<FChunkParams>& ChunkParams) const
{
	const auto World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	//Spawn actor
	const auto Chunk = ChunkParams->OriginalChunk;
	TWeakObjectPtr<AChunkRMCActor> ActorPtr = Chunk->ChunkMeshActor;
	const auto SpawnLocation = FVector(Chunk->GridPosition) * VoxelGenerator->GetChunkAxisSize();

	FAttachmentTransformRules ActorAttachmentRules = FAttachmentTransformRules::KeepWorldTransform;
	if (!ChunkParams->WorldTransform)
	{
		ActorAttachmentRules = FAttachmentTransformRules::KeepRelativeTransform;
	}

	if (ActorPtr == nullptr)
	{
		// If there is no actor spawn new one.
		ActorPtr = World->SpawnActor<AChunkRMCActor>(AChunkRMCActor::StaticClass(), SpawnLocation,
		                                             FRotator::ZeroRotator);

		if (!ActorPtr.IsValid() || !ChunkParams->SpawnerPtr.IsValid())
		{
			return;
		}

		ActorPtr->AttachToActor(ChunkParams->SpawnerPtr.Get(), ActorAttachmentRules);
	}
	else
	{
		if (!ActorPtr.IsValid())
		{
			return;
		}

		// If actor exists, ensure correct location
		if (!ChunkParams->WorldTransform)
		{
			ActorPtr->SetActorRelativeLocation(SpawnLocation);
		}
		else
		{
			ActorPtr->SetActorLocation(SpawnLocation);
		}
	}

	Chunk->ChunkMeshActor = ActorPtr;


	ActorPtr->PrepareMesh();
	const auto RealtimeMesh = ActorPtr->RealtimeMeshComponent->GetRealtimeMeshAs<
		URealtimeMeshSimple>();

	// Fill actor with mesh
	// Now we create the section group, since the stream set has polygroups, this will create the sections as well
	RealtimeMesh->CreateSectionGroup(ActorPtr->GroupKey, StreamSet);

	for (const auto VoxelId : LocalVoxelTable)
	{
		// Add voxel materials to mesh
		const auto MaterialId = VoxelId.Value;
		const auto VoxelType = VoxelGenerator->GetVoxelTypeById(VoxelId.Key);
		RealtimeMesh->SetupMaterialSlot(MaterialId, VoxelType.Key, VoxelType.Value.Material);

		const auto Key = FRealtimeMeshSectionKey::CreateForPolyGroup(ActorPtr->GroupKey, MaterialId);

		/**
		* This code may cause debugger trigger when closing editor while collision is still generating.
		* It seems that collider is created on async thread, but currently there is no way to wait for its creation or kill it.
		* Async thread becomes invalid and wrong memory is access which causes the trigger.
		* It does not break the execution, you can continue after breakpoint trigger, it will only log to console.
		* For futher details you may join discord channel: https://discord.gg/KGvBBTv 
		*/

		// Generate collider
		RealtimeMesh->UpdateSectionConfig(Key, FRealtimeMeshSectionConfig(
			                                  ERealtimeMeshSectionDrawType::Static, MaterialId),
		                                  true);
	}
}
