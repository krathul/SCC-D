import sys
import math
import subprocess
import networkx as nx


def build_graph(file_path):
    graph = nx.DiGraph()
    with open(file_path, 'r') as file:
        for line in file:
            node1, node2 = map(int, line.strip().split())
            graph.add_edge(node1, node2)
    return len(graph.nodes), len(graph.edges)


def in_helper(batches):
    d_log = open("log/d_in", "w")
    s_log = open("log/s_in", "w")

    for batch in range(batches):
        d_log.write("test/update%d\n"%(batch+1))
        n_nodes,n_edges = build_graph("test/input%d"%(batch+1))
        s_log.write("test/input%d "%(batch+1) + str(n_nodes) + " " + str(n_edges) + "\n")


def splitter2(update_file, edges, inc=0.1):
    update_file.seek(0)
    batches = int(1/inc)
    for i in range(batches):
        subprocess.run(["cp", "test/input", "test/input%d"%(i+1)])

    for batch in range(batches):
        with open("test/update%d"%(batch+1), "w") as batch_file:
            for i in range(math.floor(batch*inc*edges), math.floor((batch+1)*inc*edges)):
                line = update_file.readline()
                batch_file.write("a " + line)
                for j in range(batch, batches):
                    with open("test/input%d"%(j+1), "a") as input_file:
                        input_file.write(line)

    in_helper(batches)


def splitter():
    dataset = sys.argv[1]
    split = 0.9

    input = open("test/input", "w")
    empty = open("test/empty", "w")
    empty.close()
    update = open("test/update", "w+")

    with open(dataset) as data:
        vertices, edges = map(int, data.readline()[1:].split(','))

        for i in range(math.floor(edges*split)):
            input.write(data.readline())
        for rem_line in data:
            update.write(rem_line)

    input.close()
    splitter2(update, edges-math.floor(edges*split))
    update.close()
    return vertices, math.floor(edges*split)


print(splitter())
