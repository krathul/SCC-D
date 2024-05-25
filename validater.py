from sys import argv
import networkx as nx

global graph


def build_graph(initial_file_path):
    global graph
    graph = nx.DiGraph()
    with open(initial_file_path, 'r') as file:
        for line in file:
            node1, node2 = map(int, line.strip().split())
            graph.add_edge(node1, node2)
    return graph


def update_graph(operations_file_path):
    global graph
    with open(operations_file_path, 'r') as file:
        for line in file:
            node1, node2 = line.strip().split()
            node1, node2 = int(node1), int(node2)
            graph.add_edge(node1, node2)
    return graph


def find_scc():
    global graph
    print(nx.number_strongly_connected_components(graph))


# Example usage:
initial_file_path = argv[1]  # Replace with the path to your initial file
operations_file_path = argv[2]  # Replace with the path to your operations file

print("creating graph from file", initial_file_path)
graph = build_graph(initial_file_path)
print("Initial Graph Nodes:", len(graph.nodes))
print("Initial Graph Edges:", len(graph.edges))
find_scc()

print("updating graph from file", operations_file_path)
graph = update_graph(operations_file_path)
print("\nUpdated Graph Nodes:", len(graph.nodes))
print("Updated Graph Edges:", len(graph.edges))
find_scc()
