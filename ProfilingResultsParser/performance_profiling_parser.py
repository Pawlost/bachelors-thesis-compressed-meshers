import matplotlib.pyplot as plt
import generate_charts_from_log as logs
import generate_charts_from_per_scenario as scenarios

def generate_voxel_meshing_performance(output_file, title, multi_array):    
    plt.figure(figsize=(12, 6))

    plt.scatter(multi_array[1], multi_array[0], color='red', marker='o', s=90)

    offset=0.05

    for i in range(len(multi_array[0])):
        plt.text(multi_array[1][i] + offset, multi_array[0][i], multi_array[2][i],
        va='center')
        
    plt.scatter([0.0], [4], marker='o', s=90)
    plt.text(0.0 + offset, 4, "Ideal point", va='center')
    
    plt.xlim(left=0, right=3.5)
    plt.xlabel('Mean time of Voxel Meshing [ms]')
    plt.ylabel('Vertices')
    plt.title(title)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{output_file}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {output_file}")

SCENARIO_COUNT = 22

scattegraph_array=[]
scattegraph_array.append(logs.generate_charts_from_log('./Data/CompressedMesherDemo.log', SCENARIO_COUNT))
scattegraph_array.append(scenarios.generate_charts_interate_logs('./Data/PerScenario', SCENARIO_COUNT))
scattegraph_array.append(["VoxelPlugin Meshing", "Run Directional Meshing from VoxelGrid", "Run Directional Meshing from RLECompression"])

generate_voxel_meshing_performance('meshing_performance.svg', "Efficiency of Voxel Meshing Algorithms", scattegraph_array)