#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "summarizer.h"
#include "utils.h"

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")("debug", "debug flag")
    ("input", po::value<std::string>(), "Input edgelist file")
    ("dataset", po::value<std::string>(), "Dataset name")
    ("delimeter", po::value<char>()->default_value('\t'), "Delimeter of edgelist file")
    ("comment", po::value<char>()->default_value('#'), "Comment of edgelist file")
    ("b", po::value<int>()->default_value(8), "Number of bands of LSH")
    ("seed", po::value<int>()->default_value(0), "Random generator seed")
    ("turn", po::value<int>()->default_value(20), "Number of iteration");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }
    bool debug = vm.count("debug");
    if (!vm.count("input"))
    {
        printf("No input file is specified!\n");
        return 2;
    }

    printf("-------------------\n");
    printf("Parameter settings:\n");
    std::string path = vm["input"].as<std::string>();
    std::string dataset = vm["dataset"].as<std::string>();
    char comment = vm["comment"].as<char>();
    char delimeter = vm["delimeter"].as<char>();
    int b = vm["b"].as<int>();
    int seed = vm["seed"].as<int>();
    int turn = vm["turn"].as<int>();
    printf("Input file: %s\n", path.c_str());
    printf("Number of band: %d\n", b);
    printf("Seed: %d\n", seed);
    printf("Iteration turn: %d\n", turn);
    printf("-------------------\n");

    Graph graph = readEdgelist(path, comment, delimeter, true);

    DPGS model(dataset, graph, b, seed, debug);
    model.run(turn);
    model.saveResult(dataset);

    return 0;
}