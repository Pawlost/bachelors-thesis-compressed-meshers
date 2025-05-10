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

def generate_vertice_svg_charts(rle_vertices, grid_vertices, voxelplugin_vertices, title, file_path, start_index, end_index, font_size_bar=3, font_size_text=14):
    
    bar_width = 0.3
    
    temp_rle_vertices = rle_vertices[start_index:end_index]
    temp_grid_vertices = grid_vertices[start_index:end_index]
    temp_voxelplugin_vertices = voxelplugin_vertices[start_index:end_index]
    x_values = range(start_index + 1, start_index + len(temp_rle_vertices) + 1)
    
    x1 = [x - bar_width for x in x_values] 
    x2 = [x for x in x_values]            
    x3 = [x + bar_width for x in x_values]
        
    plt.figure(figsize=(14, 6))

    # ------------------------------
    bars1 = plt.bar(x1, temp_rle_vertices, width=bar_width, label='RLE-based RDM')
    plt.bar_label(bars1, fontsize=font_size_bar)
    
    bars2 = plt.bar(x2, temp_voxelplugin_vertices, width=bar_width, label='Voxel Plugin')
    plt.bar_label(bars2, fontsize=font_size_bar)
    
    bars3 = plt.bar(x3, temp_grid_vertices, width=bar_width, label='Grid-based RDM' )
    plt.bar_label(bars3, fontsize=font_size_bar)
    
    plt.xticks(x_values, fontsize=font_size_text)
    plt.yticks(fontsize=font_size_text)
    plt.xlabel('Scenario Number', fontsize=font_size_text+1)
    plt.ylabel('Vertices', fontsize=font_size_text+1)
    plt.title(title, fontsize=font_size_text+2)
    plt.legend(fontsize=font_size_text)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {file_path}")
#-----------------------

def generate_sparsity_chart(opaque_arr, empty_arr, title, file_path, start_index, end_index, font_size=14):
    
    # Create a voxel sparsity plot ------------
    
    temp_opaque = opaque_arr[start_index:end_index]
    temp_empty =  empty_arr[start_index:end_index]
    x_values = range(start_index + 1, start_index + 1 + len(temp_opaque))
    
    plt.figure(figsize=(7, 5))
    
    # Plot
    plt.bar(x_values, temp_opaque, label='Opaque voxels')
    plt.bar(x_values, temp_empty, bottom=temp_opaque, label='Empty voxels')
    
    plt.xticks(x_values, fontsize=font_size)
    plt.yticks(fontsize=font_size)
    plt.xlabel('Scenario Number', fontsize=font_size+1)
    plt.ylabel('Voxel count', fontsize=font_size+1)
    plt.title(title, fontsize=font_size+2)
    plt.legend(fontsize=font_size)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

def generate_memory_chart(memory_arr, title, file_path, start_index, end_index, modify_legend=False, font_size=14):
    # Create a memory plot ---------
    plt.figure(figsize=(10, 5))
    plt.axhline(y=262144, linewidth=5, color='b', linestyle='-', label='Voxel Grid Memory (262144)')
    temp_arr = memory_arr[start_index:end_index]
    x_values = range(start_index + 1, start_index + 1 + len(temp_arr))

    bars = plt.bar(x_values, temp_arr, color='skyblue', label="RLE Compressed")
    plt.bar_label(bars)

        
    plt.xticks(x_values, fontsize=font_size)
    plt.yticks(fontsize=font_size)
    plt.xlabel('Scenario Number', fontsize=font_size+1)
    plt.ylabel('Voxel Model Memory (bytes)', fontsize=font_size+1)
    plt.title(title, fontsize=font_size+2)
    leg = plt.legend(fontsize=font_size)
    
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
    
    generate_sparsity_chart(scenario_opaque_voxels_arr, scenario_transparent_voxels_arr, 'Voxel Ratio chart for group 1', './Output/voxel_sparsity_plot_group1.svg', 0, 17, 13)
    generate_sparsity_chart(scenario_opaque_voxels_arr, scenario_transparent_voxels_arr, 'Voxel Ratio chart for group 2', './Output/voxel_sparsity_plot_group2.svg', 17, 22, 13)
    generate_sparsity_chart(scenario_opaque_voxels_arr, scenario_transparent_voxels_arr, 'Voxel Ratio chart for group 3', './Output/voxel_sparsity_plot_group3.svg', 22, scenario_count, 13)

    generate_memory_chart(scenario_memory_arr, 'Voxel Model Memory Usage for group 1', './Output/voxel_memory_plot_group1.svg', 0, 17, 12)
    generate_memory_chart(scenario_memory_arr, 'Voxel Model Memory Usage for group 2', './Output/voxel_memory_plot_group2.svg', 17, 22, True, 11)
    generate_memory_chart(scenario_memory_arr, 'Voxel Model Memory Usage for group 3', './Output/voxel_memory_plot_group3.svg', 22, scenario_count, 11)

    memory_mean = (scenario_memory_arr.mean()/262144) * 100
    
    print(f"Averge RLE percentage compression to voxel grid: {memory_mean} %")

    print("Memory SVG graph saved as 'voxel_memory_plot.svg'")

    generate_vertice_svg_charts(scenario_vertices_rle_arr, scenario_vertices_grid_arr, scenario_vertices_voxelplugin_arr, 'Vertices in Group 1', './Output/vertices_plot_group1.svg', 0, 17)
    generate_vertice_svg_charts(scenario_vertices_rle_arr, scenario_vertices_grid_arr, scenario_vertices_voxelplugin_arr, 'Vertices in Group 2', './Output/vertices_plot_group2.svg', 17, 22, 10)
    generate_vertice_svg_charts(scenario_vertices_rle_arr, scenario_vertices_grid_arr, scenario_vertices_voxelplugin_arr, 'Vertices in Group 3', './Output/vertices_plot_group3.svg', 22, scenario_count, 6)


    return [scenario_vertices_voxelplugin_arr.mean(), scenario_vertices_rle_arr.mean(), scenario_vertices_grid_arr.mean()]