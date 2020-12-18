# DPGS

This repository implements a graph summarization algorithm. The source code is written in C++, and provide a simple wrapper for Python.

## Run from C++
### Input file format
You only need to provide a graph edgelist file in the following format. The first line contains the number of nodes, and each of the follwing line contains an edge given by two nodes. Node indices should be in the range of $[0, N-1]$.

### Build & Run
To build the program, run `make compile`. You need to have a compiler with C++11 compatibility. You need to install boost_program_options library as well.

After building, you can execute `./main` to run the program. Options include:
* `dataset`: Dataset name. All the output files will be placed in `output/{dataset}` directory.
* `input`: Input edgelist file. The first line is the number of nodes $N$, and the rest of the file stores the edges (one edge each line). The node ids should in the range of $[0, N-1]$.
* `--delimeter`: Delimeter of edgelist file, default `\t`.
* `--comment`: Comment character of edgelist file, default `#`.
* `--b`: Maximum number of bands of LSH, default `8`.
* `--seed`: RNG seed, default `42`.
* `--turn`: Iteration turn of algorithm, default `30`.
* `--debug`: If specified, add debug information in log file.

You can also run `run.sh {dataset}` to run with default parameter. In this way, the graph edgelist should be placed in `data/{dataset}.txt`.

For more details about options, see the source code.

### Output
The output contains three parts:
* `degrees.txt`, which stores the degree of nodes.
* `summary.edgelist`, which stores the summary graph.
* `supernodes.txt`, which stores the supernode information.

To convert the result to python-friendly format, you can run `python process_result.py {dataset}`.

## Run from Python (recommended)
We provide python binding for DPGS using pybind11. You are able to call the program from Python!

### Input file format
In this case, you only need to provide a graph adjacency file, in scipy's `.npz` format. This is much faster than reading edgelist at C++ side.

### Build & Run
To build the program, run `make py`. You need to have pybind11 installed (just run `conda install pybind11`), and have a compiler compatible to C++11 standard.

After building, you can execute `python main.py` to run the program. Options include:
* `dataset`: Dataset name. All the output files will be placed in `output/{dataset}` directory.
* `adj`: Input adjacency matrix file, in scipy's `.npz` format.
* `--turn`: Iteration turn of algorithm, default `30`.
* `--b`: Maximum number of bands of LSH, default `8`.
* `--seed`: RNG seed, default `42`.
* `--debug`: If specified, add debug information in log file.

### Output
The only output file is `nodes_dict.pkl`, which contains supernodes information. Any other post-processing can be done based on it.
