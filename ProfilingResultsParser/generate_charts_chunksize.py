import matplotlib.pyplot as plt
import pandas as pd
import generate_charts_from_per_scenario as scenarios

def generate_total_time_chart(output_file, title, dictionary, chunk_sizes):
    
    for key, value in dictionary.items():
        plt.plot(chunk_sizes, value,  label=key)
    
    plt.xlabel(f'Chunk size')
    plt.ylabel('time [ms]')
    plt.title(title)
    plt.grid(True)
    plt.legend()
    plt.xticks(chunk_sizes)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{output_file}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {output_file}")
    
def generate_chunk_size_charts():
    
    chunk_size_scenarios = [1, 5, 8, 17]
    chunk_sizes = [8, 16, 32, 64, 128]
    
    for scenario in chunk_size_scenarios:
        voxelplugin_total_dict = {}
        scenarios.init_total_dictionary(voxelplugin_total_dict)
        
        rle_total_dict = {}
        scenarios.init_total_dictionary(rle_total_dict)
        
        grid_total_dict = {}
        scenarios.init_total_dictionary(grid_total_dict)
        
        for chunksize in chunk_sizes: 
            scenarios.read_csv_file_dictionary(f"./Data/ChunkSizeSubScenarios/{scenario}/{chunksize}/VoxelPlugin_Meshing_Total.csv", voxelplugin_total_dict)
            scenarios.read_csv_file_dictionary(f"./Data/ChunkSizeSubScenarios/{scenario}/{chunksize}/RLE_Meshing_Total.csv", rle_total_dict)
            scenarios.read_csv_file_dictionary(f"./Data/ChunkSizeSubScenarios/{scenario}/{chunksize}/Grid_Meshing_Total.csv", grid_total_dict)
            
        generate_total_time_chart(f"total_voxelplugin_plot_chunksize_{scenario}.svg", f"Total Voxel Meshing time for VoxelPlugin in {scenario} Scenario", voxelplugin_total_dict, chunk_sizes)
        generate_total_time_chart(f"total_rle_plot{scenario}.svg", f"Total Voxel Meshing time for RLECompression in {scenario} Scenario", rle_total_dict, chunk_sizes)
        generate_total_time_chart(f"total_grid_plot{scenario}.svg", f"Total Voxel Meshing time for VoxelGrid in {scenario} Scenario", grid_total_dict, chunk_sizes)