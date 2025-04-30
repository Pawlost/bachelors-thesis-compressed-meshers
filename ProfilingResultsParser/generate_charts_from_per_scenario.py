import re
import os
import matplotlib.pyplot as plt

def generate_fps_svg_chart(array, title, file_path):
    x_values = range(1, len(array) + 1)
    plt.bar(x_values, array, color='skyblue')
    plt.xlabel('Scenario Number')
    plt.ylabel('FPS')
    plt.title(title)
    plt.grid(True)
    plt.xticks(x_values)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(file_path, format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {file_path}")

def read_fps_file(log_file, fps_arr):
    if os.path.exists(log_file):
        with open(log_file, 'r') as file:
            content = file.read()
            matches = re.findall(r'for\s+a\s+([\d.]+)\s+FPS\s+average', content)
            fps_arr.append(float(matches[0]))       
    else:
        fps_arr.append(0.0)
    
def generate_charts_interate_logs(folder_path, scenario_count):
    print("Reading scenario logs")
    
    grid_fps_arr = []
    rle_fps_arr = []
    voxelplugin_fps_arr = []
        
    for i in range(1, scenario_count):
        print(f"Reading Scenario{i}")
        read_fps_file(f"{folder_path}/{i}/Grid_FPS.log", grid_fps_arr)
        read_fps_file(f"{folder_path}/{i}/RLE_FPS.log", rle_fps_arr)
        read_fps_file(f"{folder_path}/{i}/VoxelPlugin_FPS.log", voxelplugin_fps_arr)
         
    print(grid_fps_arr)
    print(rle_fps_arr)    
    print(voxelplugin_fps_arr)
    
    generate_fps_svg_chart(grid_fps_arr, "Run Directional Meshing FPS per Scenario", "./Output/grid_fps_plot.svg")
    generate_fps_svg_chart(rle_fps_arr, "Run Directional Meshing FPS per Scenario", "./Output/rle_fps_plot.svg")
    generate_fps_svg_chart(voxelplugin_fps_arr, "Run Directional Meshing FPS per Scenario", "./Output/voxelplugin_fps_plot.svg")
