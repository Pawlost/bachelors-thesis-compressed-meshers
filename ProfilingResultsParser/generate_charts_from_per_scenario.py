import re
import os
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plot_phase_bars(dictionary, start_index, end_index):
    plt.figure(figsize=(18, 6))

    phase1 = np.array(dictionary[1])[start_index:end_index]
    phase2 = np.array(dictionary[2])[start_index:end_index]
    phase3 = np.array(dictionary[3])[start_index:end_index]
    
    x_values = range(start_index + 1, start_index + 1 + len(phase1))

    # Plot
    plt.bar(x_values, phase1, color='b', label='Inserting vertices to UE Buffer')
    plt.bar(x_values, phase2, color='y',bottom=phase1, label='Voxel Meshing')
    plt.bar(x_values, phase3, color='g', bottom=phase1 + phase2, label='Other timings')
    plt.xticks(x_values)
    
def plot_octree(dictionary, start_index, end_index):
    plt.figure(figsize=(18, 6))
    
    phase1 = np.array(dictionary[1])[start_index:end_index]
    phase2 = np.array(dictionary[2])[start_index:end_index]
    phase3 = np.array(dictionary[3])[start_index:end_index]
    phase4 = np.array(dictionary[4])[start_index:end_index]

    x_values = range(start_index + 1, start_index + 1 + len(phase1))
    
    plt.bar(x_values, phase4, label='Octree sampling')
    plt.bar(x_values, phase1, color='b', bottom=phase4, label='Inserting vertices to UE Buffer')
    plt.bar(x_values, phase2, color='y', bottom=phase1+phase4, label='Voxel Meshing')
    plt.bar(x_values, phase3, color='g', bottom=phase1+phase2+phase4, label='Other timings')

def generate_phase_svg_chart(output_file, title):
    plt.xlabel('Scenario Number')
    plt.ylabel('time [ms]')
    plt.title(title)
    plt.legend()

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{output_file}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {output_file}")
    
def generate_fps_svg_chart(output_file, title, array):
    x_values = range(1, len(array) + 1)
    plt.bar(x_values, array, color='skyblue')
    plt.xlabel('Scenario Number')
    plt.ylabel('FPS')
    plt.title(title)
    plt.xticks(x_values)
    plt.ylim(bottom=0, top=450000)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{output_file}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {output_file}")

def read_fps_file(log_file, fps_arr):
    if os.path.exists(log_file):
        with open(log_file, 'r') as file:
            content = file.read()
            matches = re.findall(r'for\s+a\s+([\d.]+)\s+FPS\s+average', content)
            fps_arr.append(float(matches[0]))       
    else:
        fps_arr.append(0.0)
        
def read_csv_file_dictionary(csv_file, dictionary):
    if os.path.exists(csv_file):

        # Load the CSV
        df = pd.read_csv(csv_file)

        # Optional: convert times to floats if needed
        df['StartTime'] = df['StartTime'].astype(float)
        df['EndTime'] = df['EndTime'].astype(float)

        # Calculate durations
        df['Duration'] = (df['EndTime'] - df['StartTime']) * 1000
        dictionary['mean'].append( df['Duration'].mean())
        dictionary['median'].append( df['Duration'].median())
        dictionary['min'].append( df['Duration'].min())
        dictionary['max'].append( df['Duration'].max())
    else:
        dictionary['mean'].append(0)
        dictionary['median'].append(0)
        dictionary['min'].append(0)
        dictionary['max'].append(0)

def init_total_dictionary(dictionary):
    dictionary['mean'] = []
    dictionary['median'] = []
    dictionary['min'] = []
    dictionary['max'] = []
    
    
def init_phase_dictionary(dictionary):
    dictionary[0] = []
    dictionary[1] = []
    dictionary[2] = []
    dictionary[3] = []
    
def read_phase(csv_file, array):
    if os.path.exists(csv_file):

        # Load the CSV
        df = pd.read_csv(csv_file)

        # Optional: convert times to floats if needed
        df['StartTime'] = df['StartTime'].astype(float)
        df['EndTime'] = df['EndTime'].astype(float)

        # Calculate durations
        df['Duration'] = (df['EndTime'] - df['StartTime']) * 1000
        array.append( df['Duration'].sum())
        print(f"{csv_file}: {array[-1]}")
    else:
        array.append(0)
    
def generate_total_time_chart(output_file, title, dictionary):
    
    for key, value in dictionary.items():
        x_values = range(1, len(value) + 1)
        plt.plot(x_values, value,  label=key)
    
    plt.xlabel('Scenario Number')
    plt.ylabel('time [ms]')
    plt.title(title)
    plt.grid(True)
    plt.legend()
    plt.xticks(x_values)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{output_file}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {output_file}")
    
def read_phases(partial_path, phase_dict):
    read_phase(f"{partial_path}_Total.csv", phase_dict[0])
    read_phase(f"{partial_path}_Buffer.csv", phase_dict[1])
    read_phase(f"{partial_path}.csv", phase_dict[2])
    phase_dict[3].append(phase_dict[0][-1] - phase_dict[1][-1] - phase_dict[2][-1])
    
    
def generate_charts_interate_logs(folder_path, scenario_count):
    print("Reading scenario logs")
    
    grid_fps_arr = []
    rle_fps_arr = []
    voxelplugin_fps_arr = []
    #
    voxelplugin_total_dict = {}
    voxelplugin_phase_dict = {}

    init_total_dictionary(voxelplugin_total_dict)
    init_phase_dictionary(voxelplugin_phase_dict)
    voxelplugin_phase_dict[4] = []   
    
    #
    rle_total_dict = {}
    rle_phase_dict = {}
    init_total_dictionary(rle_total_dict)
    init_phase_dictionary(rle_phase_dict)
    #
    grid_total_dict = {}
    grid_phase_dict = {}
    init_total_dictionary(grid_total_dict)
    init_phase_dictionary(grid_phase_dict)
        
    for i in range(1, scenario_count):
        print(f"Reading Scenario{i}")
        read_fps_file(f"{folder_path}/{i}/Grid_FPS.log", grid_fps_arr)
        read_fps_file(f"{folder_path}/{i}/RLE_FPS.log", rle_fps_arr)
        read_fps_file(f"{folder_path}/{i}/VoxelPlugin_FPS.log", voxelplugin_fps_arr)
        
        #CSV
        read_csv_file_dictionary(f"{folder_path}/{i}/VoxelPlugin_Meshing_Total.csv", voxelplugin_total_dict)
        read_csv_file_dictionary(f"{folder_path}/{i}/RLE_Meshing_Total.csv", rle_total_dict)
        read_csv_file_dictionary(f"{folder_path}/{i}/Grid_Meshing_Total.csv", grid_total_dict)
        
        #
        read_phases(f"{folder_path}/{i}/Grid_Meshing", grid_phase_dict)
        read_phases(f"{folder_path}/{i}/RLE_Meshing", rle_phase_dict)
        read_phases(f"{folder_path}/{i}/VoxelPlugin_Meshing", voxelplugin_phase_dict)
        read_phase(f"{folder_path}/{i}/VoxelPlugin_Meshing_Octree.csv", voxelplugin_phase_dict[4])
        voxelplugin_phase_dict[3][-1] -= voxelplugin_phase_dict[4][-1]
    
    
    
    generate_fps_svg_chart("fps_grid_plot.svg", "Run Directional Meshing FPS per Scenario", grid_fps_arr)
    generate_fps_svg_chart("fps_rle_plot.svg", "RLE Run Directional Meshing FPS per Scenario", rle_fps_arr)
    generate_fps_svg_chart("fps_voxelplugin_plot.svg", "Voxel Plugin FPS per Scenario", voxelplugin_fps_arr)
    
    generate_total_time_chart("total_voxelplugin_plot.svg", "Total Voxel Meshing time for VoxelPlugin per Scenario", voxelplugin_total_dict)
    generate_total_time_chart("total_rle_plot.svg", "Total Voxel Meshing time for RLECompression per Scenario", rle_total_dict)
    generate_total_time_chart("total_grid_plot.svg", "Total Voxel Meshing time for VoxelGrid per Scenario", grid_total_dict)
    
    plot_phase_bars(grid_phase_dict, 0, 15)
    generate_phase_svg_chart("phase_grid_plot_group1.svg","Group 1 - Voxel Meshing Phases in VoxelGrid per Scenario")
    
    plot_phase_bars(grid_phase_dict, 17, 22)
    generate_phase_svg_chart("phase_grid_plot_group2.svg","Group 2 - Voxel Meshing Phases in VoxelGrid per Scenario")
    
    plot_phase_bars(grid_phase_dict, 22, scenario_count)
    generate_phase_svg_chart("phase_grid_plot_group3.svg","Group 3 - Voxel Meshing Phases in VoxelGrid per Scenario")
    
   
    plot_phase_bars(rle_phase_dict, 0, 15)
    generate_phase_svg_chart("phase_rle_plot_group1.svg","Group 1 - Voxel Meshing Phases in RLECompression per Scenario")
  
    plot_phase_bars(rle_phase_dict, 17, 22)
    generate_phase_svg_chart("phase_rle_plot_group2.svg","Group 2 - Voxel Meshing Phases in RLECompression per Scenario")
    
    plot_phase_bars(rle_phase_dict, 22, scenario_count)
    generate_phase_svg_chart("phase_rle_plot_group3.svg","Group 3 - Voxel Meshing Phases in RLECompression per Scenario") 
    

    plot_octree(voxelplugin_phase_dict, 0, 15)
    generate_phase_svg_chart("phase_voxelplugin_plot_group1.svg","Group 1 - Voxel Meshing Phases in VoxelPlugin per Scenario")
    
    plot_octree(voxelplugin_phase_dict, 17, 22)
    generate_phase_svg_chart("phase_voxelplugin_plot_group1.svg","Group 2 - Voxel Meshing Phases in VoxelPlugin per Scenario")
    
    plot_octree(voxelplugin_phase_dict, 22, scenario_count)
    generate_phase_svg_chart("phase_voxelplugin_plot_group1.svg","Group 3 - Voxel Meshing Phases in VoxelPlugin per Scenario")

    return [np.array(voxelplugin_total_dict['mean']).mean(), np.array(rle_total_dict['mean']).mean(), np.array(grid_total_dict['mean']).mean()]

