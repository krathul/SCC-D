import subprocess

if __name__ == '__main__':

    log = open("test/log.txt", "w")
    with open("log/s_in") as input:
        for line in input:
            graph_file, n_nodes, n_edges = line.strip('\n').split(' ')
            subprocess.run(["./build/StaticSCC", graph_file, n_nodes, n_edges])
