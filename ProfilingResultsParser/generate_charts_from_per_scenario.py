import re
import os
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from matplotlib.patches import Patch


def dictionary_to_phases(dictionary, index, start_index, end_index):
    return np.array(dictionary[index])[start_index:end_index]


def generate_phase_svg_chart(voxelplugin_dictionary, rle_dictionary, grid_dictionary, title, file_path, start_index, end_index, fig_width, fig_height=6, font_size=14):
    
    plt.figure(figsize=(fig_width, fig_height))

    voxelplugin_phase1 = dictionary_to_phases(voxelplugin_dictionary, 1, start_index, end_index)
    voxelplugin_phase2 = dictionary_to_phases(voxelplugin_dictionary, 2, start_index, end_index)
    voxelplugin_phase3 = dictionary_to_phases(voxelplugin_dictionary, 3, start_index, end_index)
    voxelplugin_phase4 = dictionary_to_phases(voxelplugin_dictionary, 4, start_index, end_index)
    
    bar_width = 0.28
    
    x_values = range(start_index + 1, start_index + len(voxelplugin_phase1) + 1)
    
    offset = 0.02
    x1 = [x - bar_width - offset for x in x_values] 
    x2 = [x for x in x_values]            
    x3 = [x + bar_width + offset for x in x_values]
    
    plt.bar(x1, voxelplugin_phase4, color="r", width=bar_width)
    plt.bar(x1, voxelplugin_phase1, width=bar_width, color="b", bottom=voxelplugin_phase4)
    plt.bar(x1, voxelplugin_phase2, width=bar_width, color="y", bottom=voxelplugin_phase1+voxelplugin_phase4)
    bars1 = plt.bar(x1, voxelplugin_phase3, width=bar_width, color="g", bottom=voxelplugin_phase1+voxelplugin_phase2+voxelplugin_phase4)
    bars1_labels = ["VP"] * len(x1)
    plt.bar_label(bars1, labels=bars1_labels)

    rle_phase1 = dictionary_to_phases(rle_dictionary, 1, start_index, end_index)
    rle_phase2 = dictionary_to_phases(rle_dictionary, 2, start_index, end_index)
    rle_phase3 = dictionary_to_phases(rle_dictionary, 3, start_index, end_index)

    plt.bar(x2, rle_phase1, width=bar_width, color="b")
    plt.bar(x2, rle_phase2, width=bar_width, color="y", bottom=rle_phase1)
    bars2 = plt.bar(x2, rle_phase3, width=bar_width, color="g", bottom=rle_phase1 + rle_phase2)
    bars2_labels = ["RLE"] * len(x2)
    plt.bar_label(bars2, labels=bars2_labels)
    
    grid_phase1 = dictionary_to_phases(grid_dictionary, 1, start_index, end_index)
    grid_phase2 = dictionary_to_phases(grid_dictionary, 2, start_index, end_index)
    grid_phase3 = dictionary_to_phases(grid_dictionary, 3, start_index, end_index)

    plt.bar(x3, grid_phase1, width=bar_width, color="b")
    plt.bar(x3, grid_phase2, width=bar_width, color="y", bottom=grid_phase1)
    bars3 = plt.bar(x3, grid_phase3, width=bar_width, color="g", bottom=grid_phase1 + grid_phase2)
    bars3_labels = ["Grid"] * len(x3)
    plt.bar_label(bars3, labels=bars3_labels)

    plt.xticks(x_values, fontsize=font_size)
    plt.yticks(fontsize=font_size)
    plt.xlabel('Scenario Number', fontsize=font_size+1)
    plt.ylabel('time [ms]', fontsize=font_size+1)
    plt.title(title, fontsize=font_size+2)
    
    custom_legend = [Patch(color='r', label='Octree sampling'), Patch(color='g', label='Other timings'), Patch(color='y', label='Voxel Meshing'), Patch(color='b', label='Insertion into UE Buffers')]
    plt.legend(handles=custom_legend, fontsize=font_size)
    
    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{file_path}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {file_path}")
    
def plot_scenario_17():
    print("hello")

    
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
        array.append( df['Duration'].mean())
        print(f"{csv_file}: {array[-1]}")
    else:
        array.append(0)
    
def generate_total_time_chart(dictionary, output_file, title,  start_index, end_index,font_size=14):
    
    for key, value in dictionary.items():    
        temp_time = value[start_index:end_index]
        x_values = range(start_index + 1, start_index + len(temp_time) + 1)
        plt.plot(x_values, temp_time, label=key)
        
    plt.grid(True)
    plt.xticks(x_values, fontsize=font_size)
    plt.yticks(fontsize=font_size)
    plt.xlabel('Scenario Number', fontsize=font_size+1)
    plt.ylabel('time [ms]', fontsize=font_size+1)
    plt.title(title, fontsize=font_size+2)
    
    plt.legend(fontsize=font_size)

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
    
    generate_total_time_chart(rle_total_dict, "total_rle_plot_factor1.svg", "Run Direction Meshing from RLE in Group 2", 17, 22)
    generate_total_time_chart(rle_total_dict, "total_rle_plot_factor2.svg", "Total Voxel Meshing time for RLE in Group 3", 22, scenario_count)
    
    generate_phase_svg_chart(voxelplugin_phase_dict, rle_phase_dict, grid_phase_dict, "Voxel Meshing Phases Group 1", "phase_plot_group1.svg", 0, 16, 15, 9, font_size=18)
    generate_phase_svg_chart(voxelplugin_phase_dict, rle_phase_dict, grid_phase_dict, "Voxel Meshing Phases Scenario 17", "phase_plot_scenario_17.svg", 16, 17, 4)
    generate_phase_svg_chart(voxelplugin_phase_dict, rle_phase_dict, grid_phase_dict, "Voxel Meshing Phases Group 2", "phase_plot_group2.svg", 17, 22, 10)
    generate_phase_svg_chart(voxelplugin_phase_dict, rle_phase_dict, grid_phase_dict, "Voxel Meshing Phases Group 3", "phase_plot_group3.svg", 22, 37, 15, 9, font_size=18)

    return [np.array(voxelplugin_total_dict['mean']).mean(), np.array(rle_total_dict['mean']).mean(), np.array(grid_total_dict['mean']).mean()]

