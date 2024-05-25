### To build
```
$ mkdir build && cd build
$ cmake ..
// or if you want to generate a compile_commands.json for your lsp
$ cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
// generates two binaries in the build folder
```

### To run
```bash
# first download the dataset
# run preprocess.py, it extracts the archive, formats it into required representation and writes it to a new file inside the same directory
$ python preprocess.py [path to the datazip archive]

# run feeder.py, it splits the earlier extracted file to input and updates, stores in test directory. (the split ratio and batches are hard coded)
$ python feeder.py [path to the earlier extracted dataset]
# (might take some time.... ¯\_(ツ)_/¯)
# Additionally it generates logfiles written to log directory.

# run the static algorithm
$ ./build/StaticSCC [graph file] [no of vertices] [no of edges]

# run the dynamic algorithm (requires stdin input -> path to update file)
# no of vertices, here means the maximum no of vertices the final graph will have after updation
# !!because no support for vertex insertion
# no of edges -> no of edges in the original graph
$ ./build/DynamicSCC [graph file] [no of vertices] [no of edges]

# algorithms can be validated using validator.py
$ python validator.py [graph file] [update file]

```
