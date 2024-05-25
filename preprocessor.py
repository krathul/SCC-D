import sys
import subprocess
import os

if __name__ == '__main__':
    edges = {}
    vertices = {}
    vertex_cnt = 0
    edge_cnt = 0

    with open("test/temp", "w") as f:
        subprocess.run(["gunzip", "-c", sys.argv[1]], stdout=f)
    save_file = open(sys.argv[1].split('.')[0], "w")

    with open("test/temp") as input, open("test/temp1", "w") as output:
        for line in input:
            if '#' in line:
                continue

            node1, node2 = map(int, line.strip().split())

            if node1 not in vertices.keys():
                vertex_cnt += 1
                vertices[node1] = vertex_cnt - 1
            if node2 not in vertices.keys():
                vertex_cnt += 1
                vertices[node2] = vertex_cnt - 1

            if (vertices[node1], vertices[node2]) in edges.keys():
                continue
            else:
                edges[(vertices[node1], vertices[node2])] = True
                output.write(str(vertices[node1]) + "\t" + str(vertices[node2])
                             + "\n")
                edge_cnt += 1
        save_file.write("#" + str(vertex_cnt) + "," + str(edge_cnt) + "\n")

    with open("test/temp1", "r") as output:
        for line in output:
            save_file.write(line)

    os.remove("test/temp")
    os.remove("test/temp1")
    save_file.close()
