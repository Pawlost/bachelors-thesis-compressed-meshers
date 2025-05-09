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
        if array[scenario_index] == 0:
            array[scenario_index] = value
        else:
            array[scenario_index] = round((array[scenario_index]+value)/2)

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

def generate_sparsity_chart(opaque_arr, empty_arr, title, file_path, start_index, end_index):
    
    # Create a voxel sparsity plot ------------
    
    temp_opaque = opaque_arr[start_index:end_index]
    temp_empty =  empty_arr[start_index:end_index]
    x_values = range(start_index + 1, start_index + 1 + len(temp_opaque))
    
    plt.figure(figsize=(7, 5))
    
    # Plot
    plt.bar(x_values, temp_opaque, label='Opaque voxels')
    plt.bar(x_values, temp_empty, bottom=temp_opaque, label='Empty voxels')
    
    plt.xlabel('Scenario Number')
    plt.ylabel('Voxel count')
    plt.title(title)
    plt.legend()
    plt.xticks(x_values)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

def generate_memory_chart(memory_arr, title, file_path, start_index, end_index, modify_legend=False):
    # Create a memory plot ---------
    plt.figure(figsize=(10, 5))
    plt.axhline(y=262144, linewidth=5, color='b', linestyle='-', label='Voxel Grid Memory (262144)')
    temp_arr = memory_arr[start_index:end_index]
    x_values = range(start_index + 1, start_index + 1 + len(temp_arr))

    bars = plt.bar(x_values, temp_arr, color='skyblue', label="RLE Compressed")
    plt.bar_label(bars)
    plt.xlabel('Scenario Number')
    plt.ylabel('Voxel Model Memory (bytes)')
    plt.title(title)
    plt.xticks(x_values)
    
    leg = plt.legend()
    if modify_legend:
        leg.set_bbox_to_anchor((1, 0.9))
    
    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

def generate_charts_from_log(scenario_count):
    print("Reading lines.")

    directory = Path("./Data/logs")
    
    # Define regex pattern for correct log lines
    log_pattern = re.compile(r".+LogVoxelMeshingProfiling.+")

    print(scenario_count )

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
                            scenario_opaque_voxels_arr[scenario_index] = (scenario_opaque_voxels_arr[scenario_index]+opaque_voxel_count)/2
                            
                            transparent_voxel_count = int(match.group(3))
                            scenario_transparent_voxels_arr[scenario_index] = (scenario_transparent_voxels_arr[scenario_index] + transparent_voxel_count)/2
                            
            with open(file_path, 'w') as f:
                f.writelines(filtered_file_lines)
                
    print("Parsed lines from logs.")
    print("Generating graphs.")
    
    generate_sparsity_chart(scenario_opaque_voxels_arr, scenario_transparent_voxels_arr, 'Voxel Sparsity chart for group 1', './Output/voxel_sparsity_plot_group1.svg', 0, 16)
    generate_sparsity_chart(scenario_opaque_voxels_arr, scenario_transparent_voxels_arr, 'Voxel Sparsity chart for group 2', './Output/voxel_sparsity_plot_group2.svg', 17, 22)
    generate_sparsity_chart(scenario_opaque_voxels_arr, scenario_transparent_voxels_arr, 'Voxel Sparsity chart for group 3', './Output/voxel_sparsity_plot_group3.svg', 22, 36)

    generate_memory_chart(scenario_memory_arr, 'Voxel Model Memory Usage for group 1', './Output/voxel_memory_plot_group1.svg', 0, 16)
    generate_memory_chart(scenario_memory_arr, 'Voxel Model Memory Usage for group 2', './Output/voxel_memory_plot_group2.svg', 17, 22, True)
    generate_memory_chart(scenario_memory_arr, 'Voxel Model Memory Usage for group 3', './Output/voxel_memory_plot_group3.svg', 22, 36)

    print("Memory SVG graph saved as 'voxel_memory_plot.svg'")

    generate_vertice_svg_chart(scenario_vertices_rle_arr, 'Run Directional Meshing from RLE', './Output/vertices_rle_plot.svg')
    generate_vertice_svg_chart(scenario_vertices_grid_arr, 'Run Directional Meshing from Voxel Grid', './Output/vertices_grid_plot.svg')
    generate_vertice_svg_chart(scenario_vertices_voxelplugin_arr, 'VoxelPlugin Cubic Meshing', './Output/vertices_voxelplugin_plot.svg')

    return [scenario_vertices_rle_arr.mean(), scenario_vertices_grid_arr.mean(), scenario_vertices_voxelplugin_arr.mean()]