import re
import matplotlib.pyplot as plt

def match_to_scenario(match):
    return int(match.group(1).strip()) - 1

def match_line_to_arr(regex_pattern, line, array):
    match = re.search(regex_pattern, line)
    if match:
        scenario_index = match_to_scenario(match)
        value = int(match.group(2))
        array[scenario_index] = value

def generate_vertice_svg_chart(array, title, file_path):
    x_values = range(1, len(array))
    plt.bar(x_values, array, color='skyblue')
    plt.xlabel('Scenario Number')
    plt.ylabel('Vertices')
    plt.title(title)
    plt.grid(True)
    plt.xticks(x_values)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {file_path}")
#-----------------------

def generate_charts_from_log(log_file_path, scenario_count):
    print("Reading lines.")

    with open(log_file_path, 'r') as file:
        file_lines = file.readlines()

    # Define regex pattern for correct log lines
    log_pattern = re.compile(r".+LogVoxelMeshingProfiling.+")

    #Preallocate array to scenarios
    scenario_memory_arr = [0] * scenario_count
    scenario_vertices_rle_arr = [0] * scenario_count
    scenario_vertices_grid_arr = [0] * scenario_count
    scenario_vertices_voxelplugin_arr = [0] * scenario_count
    scenario_opaque_voxels_arr = [0] * scenario_count
    scenario_transparent_voxels_arr = [0] * scenario_count

    # Find matching lines in logs
    matching_lines = [line for line in file_lines if log_pattern.search(line)]

    print("Extracted lines from logs.")

    # Parse lines into array
    for line in matching_lines:
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
            
    print("Parsed lines from logs.")
    print("Generating graphs.")
    
    # Create a voxel sparsity plot ------------
    x_values = range(1, len(scenario_opaque_voxels_arr))
    
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
    plt.axhline(y=262144, linewidth=3, color='b', linestyle='-', label='y=262144')
    x_values = range(1, len(scenario_memory_arr))

    bars = plt.bar(x_values, scenario_memory_arr, color='skyblue')
    plt.bar_label(bars)
    plt.xlabel('Scenario Number')
    plt.ylabel('Voxel Model Memory (bytes)')
    plt.title('Voxel Model Memory Usage by Profiling Scenario')
    plt.grid(True)
    plt.xticks(x_values)
    
    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig('./Output/voxel_memory_plot.svg', format='svg')
    plt.close()  # Close the plot to free memory

    print("Memory SVG graph saved as 'voxel_memory_plot.svg'")

    generate_vertice_svg_chart(scenario_vertices_rle_arr, 'Run Directional Meshing from RLE', './Output/rle_vertices_plot.svg')
    generate_vertice_svg_chart(scenario_vertices_grid_arr, 'Run Directional Meshing from Voxel Grid', './Output/grid_vertices_plot.svg')
    generate_vertice_svg_chart(scenario_vertices_voxelplugin_arr, 'VoxelPlugin Cubic Meshing', './Output/voxelplugin_vertices_plot.svg')
