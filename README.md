# Run Directional Meshing – Bachelor's Thesis

![RDM](./Screenshots/RDM.PNG)

**NOTICE!**
This repository is obsolete and documents the results presented in my bachelor’s thesis. The updated version with the extended optimizations is available [here](https://github.com/Pawlost/run-directional-meshing-extended).

## Table of Contents
- [Description](#description)
 - [Project as plugin](#project-as-plugin)
- [Installation](#installation)
 - [Prerequisites](#prerequisites)
 - [Project Setup](#project-setup)
- [Known Issues (RMC Plugin)](#known-issues-rmc-plugin)
- [Usage](#usage)
 - [Data Table](#data-table)
 - [Voxel Generators](#voxel-generators)
 - [Chunk Spawners](#chunk-spawners)
 - [Characters](#characters)
 - [Scenes](#scenes)
- [Releases](#releases)
- [Contribution](#contribution)
- [License](#license)
- [Contact](#contact)

## Description
This repository contains the implementation of a new voxel meshing algorithm developed for Unreal Engine 5.4 as part of a bachelor's thesis. The project showcases Run Directional Meshing, an interactive, real-time technique for converting voxel data into polygonal meshes optimized for performance and editor integration.

### Project as plugin
The main project is `CompressedMesherDemo.project`, which contains profiling datasets in its content. The plugin with the implementation is located in the `Plugins/RunDirectionalMeshingDemo` repository and may copied from this place into your solution.

This project includes a freely usable and shareable Unreal Engine plugin located in the Plugins folder.
To enable plugin content: Content Browser → Settings → Show Plugin Content.

## Installation
### Prerequisites
Windows 11:
* [Visual Studio Installer](https://visualstudio.microsoft.com/cs/downloads/)
 * [Workloads: Game development with C++, Desktop development with C++, .NET desktop development](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine?application_version=5.4)
 * Individual Components: MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.38-17.8)
* [Jetbrains Rider](https://www.jetbrains.com/rider/) (or alternative IDE able to build and run Unreal Engine)
* [Epic games launcher](https://store.epicgames.com/en-US/download)
 * [Unreal Engine 5.4](https://www.unrealengine.com/en-US/)
 * [Fast Noise Generator Plugin](https://www.fab.com/listings/c1d444fc-54cc-4f11-8a4a-c0c41112a321)
 * [Realtime Mesh - Core](https://www.fab.com/listings/bb2e4fbb-617c-41d3-aac6-e181eddf8b3b)

Note: Linux environment is not tested

After installing all prerequisites, restart a computer to ensure all dependencies are recognized.

If .uproject files are not associated with Unreal Engine:

1. Copy `C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe` to `C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64`.
2. Run UnrealVersionSelector from the directory.
3. Navigate to the .uproject file.
4. Right-click -> Open With -> Unreal Engine.

### Project Setup

1. Clone or download the repository to a directory of your choice.
2. Right-click the `.uproject` file -> Show More Options -> Generate Visual Studio project files.
3. Open the generated `.sln` solution file in **JetBrains Rider** (or your preferred IDE able to run UE projects):
 1. Wait for the IDE to process the solution files.
 2. Ensure the **RiderLink plugin** is installed.
4. Build and run the project.

For further details, see [Usage](#usage).

**Known Issue (RMC Plugin)**
When running Unreal Engine in Debug Mode, closing the editor before chunk spawning may trigger a breakpoint in the Realtime Mesh Component (RMC) plugin. This issue does not cause a crash, resume execution should continue.

## Usage
The first scene that opens in this project is `Map_Scenario_Showcase.umap`. 

### Data table
Example tables can be found in `Plugins/RunDirectionalMeshingDemo/VoxelTypes`.

* Example voxel definitions are stored in `DT_BlockTable`.
* Use the **VoxelType** row struct to create a new voxel table.
* Add new rows, assign materials, and configure properties.

![VoxelTypeCreation](./Screenshots/VoxelTypeTableCreation.jpg)

### Voxel Generators
For example, voxel generators can be found in `Plugins/RunDirectionalMeshingDemo//ContentBlueprints/ChunkSpawners/VoxelGenerators`.

Voxel generators are unspawnable actor components that define how chunks are filled with voxels, set chunk dimensions, and determine voxel size. **Meshers** are algorithms that convert the generated voxel grid into a mesh. Currently, only **RunDirectionalMesher** is supported.

![VoxelGenerator](./Screenshots/VoxelGenerator.jpg)

### Chunk Spawners
Example tables can be found in `Plugins/RunDirectionalMeshingDemo/Blueprints/Content/ChunkSpawners`.

Chunk spawners determine the number and positions of generated chunks. To configure a chunk spawner:
1. Assign a voxel generator.
2. Modify configuration.
3. Place it in the scene.

![PreloadedSpawner](./Screenshots/PreloadedSpawner.jpg)


### Characters
This project includes two playable characters with voxel interaction capabilities:

1. BP_VoxelInteractionObserverCharacter -- Enables voxel placement and picking.
2. BP_VoxelWorldExplorer *(inherits from BP_VoxelInteractionObserverCharacter)* -- Automatically spawns chunks around the player for exploration.

They can be found in `Plugins/RunDirectionalMeshingDemo/Blueprints/Content/Characters`.

### Scenes
Example maps showcasing different voxel test scenarios can be found in `Content/ProfilingScenarios/`. The default map showcases can be found directly in `Content/`.

## Licence
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact
* Email: [mr.p.balusek@gmail.com](mailto:mr.p.balusek@gmail.com)
* Github: [Pawlost](https://github.com/Pawlost?tab=repositories)
* LinkedIn: [Pavel Balusek](https://www.linkedin.com/in/pavel-balusek-4211b4197/)