import matplotlib.pyplot as plt
import generate_charts_from_log as logs 
import generate_charts_from_per_scenario as scenarios
import generate_charts_chunksize as chunksize

def generate_voxel_meshing_performance(output_file, title, multi_array, font_size=14):    
    plt.figure(figsize=(9, 6))

    plt.scatter(multi_array[0], multi_array[1], color='red', marker='o', s=90)

    offset=0.8

    for i in range(len(multi_array[0])):
        plt.text(multi_array[0][i] + offset, multi_array[1][i], multi_array[2][i],
        va='center')
        
    plt.scatter([0.0], [4], marker='o', s=500)
    plt.text(0.0 + offset, 2000, "Ideal point", va='center')
    
    plt.xlim(left=0, right=50)
    plt.ylim(bottom=0, top=70000)
    
    plt.xticks(fontsize=font_size)
    plt.yticks(fontsize=font_size)
    plt.xlabel('Mean speed of Voxel Meshing [ms]', fontsize=font_size+1)
    plt.ylabel('Vertices', fontsize=font_size+1)
    plt.title(title, fontsize=font_size+2)

    # Save as SVG
    plt.tight_layout()  # Prevent label cutoff
    plt.savefig(f"./Output/{output_file}", format='svg')
    plt.close()  # Close the plot to free memory

    print(f"{title} SVG graph saved as {output_file}")

SCENARIO_COUNT = 37

scattegraph_array=[]
scattegraph_array.append(scenarios.generate_charts_interate_logs('./Data/PerScenario', SCENARIO_COUNT))
scattegraph_array.append(logs.generate_charts_from_log(SCENARIO_COUNT))
scattegraph_array.append(["VoxelPlugin Meshing", "Run Directional Meshing from RLE", "Run Directional Meshing from Grid"])
chunksize.generate_chunk_size_charts()

print(f"Final Graph: {scattegraph_array}")
generate_voxel_meshing_performance('meshing_performance.svg', "Efficiency of Voxel Meshing Algorithms", scattegraph_array)