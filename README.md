# summarize-cpp

This repository implements a graph summarization algorithm. The source code is written in C++, and provide simple wrapper for Python.

## Run from cpp
To build the program, run `make compile`. You need to have a compiler with C++11 compatibility. You need to install boost_program_options library as well.

After building, you can execute `./main` to run the program. Options include:
* `input`: Input edgelist file. The first line is the number of nodes $N$, and the rest of the file stores the edges (one edge each line). The node ids should in the range of $[0, N-1]$.
* `--delimeter`: Delimeter of edgelist file, default `\t`.
* `--comment`: Comment character of edgelist file, default `#`.
* `--b`: Maximum number of bands of LSH, default `8`.
* `--seed`: RNG seed, default `0`.
* `--turn`: Iteration turn of algorithm, default `30`.
* `--ratio`: Ratio of node size, default `0.0` (meaning run T turns).
* `--debug`: If specified, add debug information in log file.

You can also run `run.sh {dataset}` to run with default parameter.

For more details about options, see the source code.

The output contains three parts:
* `degrees.txt`, which stores the degree of nodes.
* `summary.edgelist`, which stores the summary graph.
* `supernodes.txt`, which stores the supernode information.

To convert the result to python-friendly format, you can run `python process_result.py {dataset}`.