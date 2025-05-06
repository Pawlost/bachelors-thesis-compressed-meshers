import re
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


def match_to_scenario(match):
    return int(match.group(1).strip()) - 1

def match_line_to_arr(regex_pattern, line, array):
    match = re.search(regex_pattern, line)
    if match:
        scenario_index = match_to_scenario(match)
        value = int(match.group(2))
        array[scenario_index] = value

def generate_vertice_svg_chart(array, title, file_path):
    x_values = range(1, len(array) + 1)
    plt.figure(figsize=(14, 6))
    bars = plt.bar(x_values, array, color='skyblue')
    plt.bar_label(bars)
    plt.xlabel('Scenario Number')
    plt.ylabel('Vertices')
    plt.title(title)
    plt.xticks(x_values)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {file_path}")
#-----------------------

def generate_charts_from_log(log_file_path, scenario_count):
    print("Reading lines.")

    directory = Path("./Data/logs")
    
    # Define regex pattern for correct log lines
    log_pattern = re.compile(r".+LogVoxelMeshingProfiling.+")

    #Preallocate array to scenarios
    scenario_memory_arr = np.zeros(scenario_count)
    scenario_vertices_rle_arr = np.zeros(scenario_count)
    scenario_vertices_grid_arr = np.zeros(scenario_count)
    scenario_vertices_voxelplugin_arr = np.zeros( scenario_count)
    scenario_opaque_voxels_arr = np.zeros( scenario_count)
    scenario_transparent_voxels_arr = np.zeros(scenario_count)

    print("Read logs.")

    for file_path in directory.glob("*"):
        
        if file_path.is_file():
            file_lines = []
            print(f"Reading file {file_path.absolute()}")
            filtered_file_lines = []
            with file_path.open("r") as f:
                file_lines = f.readlines()
                 # Parse lines into array
                for i, line in enumerate(file_lines):
                    if log_pattern.search(line): 
                        filtered_file_lines.append(line)
                        print(line)
                        match_line_to_arr(r".*Scenario name:.*RLE_Scenario(\d+); Voxel Model memory: (\d+)", line, scenario_memory_arr) 
                        match_line_to_arr(r".*Scenario name:.*RLE_Scenario(\d+); Vertices: (\d+)", line, scenario_vertices_rle_arr) 
                        match_line_to_arr(r".*Scenario name:.*Grid_Scenario(\d+); Vertices: (\d+)", line, scenario_vertices_grid_arr) 
                        match_line_to_arr(r".*Scenario name:.*VoxelPlugin_Scenario(\d+); Vertices: (\d+)", line, scenario_vertices_voxelplugin_arr)
                            
                        match = re.search(r".*Scenario name:.*VoxelPlugin_Scenario(\d+); OpaqueVoxelCount: (\d+); TransparentVoxelCount: (\d+)", line)
                        if match:
                            scenario_index = match_to_scenario(match)
                            opaque_voxel_count = int(match.group(2))
                            scenario_opaque_voxels_arr[scenario_index] = opaque_voxel_count
                            
                            transparent_voxel_count = int(match.group(3))
                            scenario_transparent_voxels_arr[scenario_index] = transparent_voxel_count
                            
            with open(file_path, 'w') as f:
                f.writelines(filtered_file_lines)
                
    print("Parsed lines from logs.")
    print("Generating graphs.")
    
    # Create a voxel sparsity plot ------------
    x_values = range(1, len(scenario_opaque_voxels_arr) + 1)
    
    # Plot
    plt.bar(x_values, scenario_opaque_voxels_arr, label='Opaque voxels')
    plt.bar(x_values, scenario_transparent_voxels_arr, bottom=scenario_opaque_voxels_arr, label='Transparent voxels')
    
    plt.xlabel('Scenario Number')
    plt.ylabel('Voxel count')
    plt.title('Voxel Sparsity chart')
    plt.grid(True)
    plt.legend()
    plt.xticks(x_values)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig('./Output/voxel_sparsity_plot.svg', format='svg')
    plt.close()  # Close the plot to free memory

    # Create a memory plot ---------
    x_values = range(1, len(scenario_memory_arr) + 1)

    plt.figure(figsize=(14, 6))
    plt.axhline(y=262144, linewidth=5, color='b', linestyle='-', label='Voxel Grid Memory (262144)')

    bars = plt.bar(x_values, scenario_memory_arr, color='skyblue', label="RLE Compressed")
    plt.bar_label(bars)
    plt.xlabel('Scenario Number')
    plt.ylabel('Voxel Model Memory (bytes)')
    plt.title('Voxel Model Memory Usage by Profiling Scenario')
    plt.xticks(x_values)
    plt.legend()
    
    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig('./Output/voxel_memory_plot.svg', format='svg')
    plt.close()  # Close the plot to free memory

    print("Memory SVG graph saved as 'voxel_memory_plot.svg'")

    generate_vertice_svg_chart(scenario_vertices_rle_arr, 'Run Directional Meshing from RLE', './Output/vertices_rle_plot.svg')
    generate_vertice_svg_chart(scenario_vertices_grid_arr, 'Run Directional Meshing from Voxel Grid', './Output/vertices_grid_plot.svg')
    generate_vertice_svg_chart(scenario_vertices_voxelplugin_arr, 'VoxelPlugin Cubic Meshing', './Output/vertices_voxelplugin_plot.svg')

    return [scenario_vertices_rle_arr.mean(), scenario_vertices_grid_arr.mean(), scenario_vertices_voxelplugin_arr.mean()]