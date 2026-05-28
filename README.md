# Brain-Connectome-Analyzer
This project models the human brain as a weighted directed graph of 90 regions based on the AAL atlas, applying Dijkstra's algorithm and BFS to trace neural signal pathways, identify critical hub regions through centrality metrics, and simulate the impact of full and partial brain lesions on network connectivity.

Description:
In this project, a C++ program for studying the human brain connectivity using graph theory algorithms is provided. The human brain will be modeled as a weighted directed graph based on the Automated Anatomical Labeling (AAL) dataset which consists of 90 different brain areas connected by their synaptic strength values.


The program takes as input a real-world brain connectivity matrix and studies neural signal transmission within the brain networks, identifying important brain hubs, and exploring the effects of changes in brain connectivity when brain lesions occur.

the link for the data:
https://osf.io/yw5vf/overview
used files are:
* Automated Anatomical Labeling) atlas (AAL) 
* Structure Connectivity Matrix (90x90 matrix)

  
Execution
./BrainGraph
On starting the program will do the following:
- Read the brain regions' labels from AAL_regions.csv
- Read the adjacency matrix from S001.csv
- Prompt user for setting default source and destination brain regions
- Show the menu of options
- Accept region inputs either by its name (Frontal_Sup_L) or by its index (1)
Data Structure
* Graph
- Adjacency list where each of the 90 vertices keeps an array of {target, weight} outgoing edges
- origAdj: The initial copy of the adjacency list that is stored and then used to revert lesions
- dead[]: Array of booleans representing vertices that have been fully lesioned (erased)
* Weight Meaning
- The higher value of the raw edge means the higher strength of the synaptic connection
- Dijkstra algorithm uses 1 / weight as a cost for traversing the edge, thus high edge values mean short (cheap) route and correspond to biological assumptions
Menu Options
1. Signal Pathfinding (Dijkstra)
2. Breadth-first search path
3. Compare Dijkstra to BFS
4. Create and print hubs list
5. Perform a total lesion
6. Perform partial lesion
7. Revert a lesioned node
8. Revert all lesions
9. Evaluate reachability after lesions
10. Print graph in .dot format
11. Print original adjacency list
12. Visualize Dijkstra's path (entire graph)
13. Set default source/destination
0. Exit
Feature Descriptions:
* 1 & 2 – Path Finding
Finds the most efficient route between the default source and destination using:
- Dijkstra’s algorithm: weighted shortest path (prefers routes with stronger connections, due to inverse weight cost)
- BFS algorithm: unweighted shortest path (minimizes hops, ignoring connection strength)
Both exclude lesioned nodes from the path search.
* 3 – Comparison of Algorithms: Dijkstra vs BFS
Runs both algorithms and compares the two results in a table containing:
- Number of hops
- Total cost of traversal (sum of 1/w)
- Total connection strength (sum of w)
- Equality of two paths
If the routes are not the same, Dijkstra finds a longer but stronger path that the BFS algorithm misses.
* 4 – Report of Hubs
Ranks all 90 brain regions based on three measures of centrality:
- Betweenness centrality: number of times the region occurs on shortest paths between other regions, acting as a bridge between brain areas
- Weighted out-degree: total strength of all outgoing connections, indicating strong signal generators
- Weighted in-degree: total strength of all incoming connections, indicating strong signal receivers
5 – Lesion (Full)
Marks a brain region as lesioned, making it unusable in all further calculations. All algorithms will skip dead nodes when searching for paths or hubs.
6 – Lesion (Partial)
Weakens the brain area by decreasing the weight of all incoming and outgoing edges by a certain factor (e.g., 0.3 will keep 30% of the connection strength).
7 & 8 – Restore
Restores either one node or all nodes, resetting their dead status and reverting the edge weights back to the way they were in the original network (using the copy stored in the backup variable origAdj).
9 – Path Integrity Check
After lesions have been applied, the feature searches the route between the default source and destination via Dijkstra's algorithm and analyzes the nodes along this path:
- Dead status
- Connectivity
- Connection strength to the next node
It reports the status of the path as either INTACT or BROKEN.
10 & 12 – Visualization as .dot File
Creates a visualization of the current brain model as a Graphviz .dot file, with colors indicating the following:
* Green — source region
* Orange — destination region
* Yellow — intermediate nodes on the highlighted path
* Red — lesioned (dead) nodes
* Light blue — all other regions
To create an image from the exported file:
dot -Tpng connectome.dot -o connectome.png
To minimize clutter, edges weighing less than 0.05 are hidden. The path borders that are highlighted have a thicker, blue line.
11. The Original Adjacency List
contains all edges in their associated weights from the original, unmodified graph’s adjacency list for all 90 nodes.
13. Change the Source and Destination
allows the modification of the standard source and destination regions used by all reachability and pathfinding calculations.

to get the graph, python code could be used and here is the code:
!pip install graphviz matplotlib
import graphviz

# Read your generated .dot file
with open("dijkstra_full.dot") as f:
    source = f.read()

graph = graphviz.Source(source)
graph.render("dijkstra_full", format="png", view=True)
