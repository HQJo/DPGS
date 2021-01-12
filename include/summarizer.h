#ifndef __SUMM_SUMMARIZER_H_
#define __SUMM_SUMMARIZER_H_

#include <fstream>

#include "lsh.h"
#include "graph.h"

class DPGS
{
    constexpr static int numPerm = 16;          // Length of MinHash signature
    constexpr static int minB = 3;              // Minimum count of LSH band
    constexpr static int C = 500;               // Maximum size of each group

public:
    DPGS(const std::string &dataset, const Graph &graph, const int maxB, const int seed = 0, bool debug = false);
    ~DPGS();
    double run(const int T = 20, const double ratio=0.5);
    void updateLSH(const int r, const int b);
    size_t mergeGroup(std::vector<int64_t> &group);
    std::pair<double, int64_t> mergeGain(const int64_t u, const int64_t v);
    void merge(const int64_t u, const int64_t v);

    bool isDeleted(const int64_t u);
    void saveResult(const std::string &dataset);
    std::vector<std::vector<int64_t>> getNodesDict() const { return nodesDict; }

private:
    std::string dataset;
    Graph graph;
    int64_t N;                                  // Original number of nodes
    int64_t M;                                  // Original number of edges
    int64_t numNode;                            // Number of nodes through summarization
    int64_t numEdge;                            // Number of edges through summarization
    std::vector<int64_t> degs;                  // Degrees array
    double origLen{0.0};
    double totalGain{0.0};
    double degPartLength{0.0};
    double initKLError;

    int maxB;                                    // Maximum number of bands
    int seed;                                    // Random seed
    double ratio;                                // Node ratio
    MinHash<int64_t> minhash;                    // MinHash object
    LSH<numPerm, int64_t> lsh;                   // LSH object
    std::vector<std::vector<int64_t>> groups;    // Groups
    std::vector<std::vector<int64_t>> nodesDict; // Supernode dict

    bool debug{false};
    std::ofstream fs;                           // Log file stream
    char buffer[128];                           // Log buffer
    void initLogger();
    void saveDegs();

    void parallelUpdateLSH(const int64_t start, const int64_t end);
    double modelLength();
    double finalKLError();
    void printInfo();
};

#endif
