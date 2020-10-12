#include <iostream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <cmath>
#include <functional>
#include <random>
#include <thread>
#include <sstream>

#include "summarizer.h"
#include "union_find.h"
#include "mdl.h"
#include "utils.h"

namespace chrono = std::chrono;

DPGS::DPGS(const std::string &dataset, const Graph &graph, const int maxB, const int seed, bool debug)
    : dataset(dataset), graph(graph), maxB(maxB), seed(seed), debug(debug)
{
    if (maxB < minB)
        this->maxB = minB;
    if (maxB > numPerm)
        this->maxB = numPerm;

    N = numNode = graph.numNode();
    degs.resize(N);
    for (int64_t u = 0; u < numNode; u++)
    {
        degs[u] = this->graph.Neighbors(u).size();
    }
    saveDegs();

    M = std::accumulate(degs.begin(), degs.end(), 0);
    M /= 2;
    numEdge = M;

    initLogger();
    printf("Graph statistics:\n");
    printf("|V|=%lu\n", N);
    printf("|E|=%lu\n", M);
    printf("-------------------\n");
    fs << "Graph statistics:\n";
    fs << "|V|=" << N << "\n";
    fs << "|E|=" << M << "\n";

    initKLError = -2 * std::accumulate(degs.begin(), degs.end(), 0.0, [](double x, int64_t y) {
        return x + xlogx(y);
    });
    auto accumulator = [](double x, int64_t y) -> double {
        return x + LN(y);
    };
    origLen = 0.0;
    // origLen += LN(N);
    // origLen += N * log2(N);
    degPartLength = std::accumulate(degs.begin(), degs.end(), 0.0, accumulator);
    // origLen += degPartLength;
    // origLen += LN(M);
    origLen += LnU(N * (N - 1) / 2, M);
    // origLen += M * LN(1);

    nodesDict.resize(numNode);
    for (int64_t i = 0; i < numNode; i++)
        nodesDict[i].push_back(i);

    minhash = MinHash<int64_t>(numPerm, seed);
}

DPGS::~DPGS()
{
    if (fs.is_open())
        fs.close();
}

void DPGS::saveDegs()
{
    const std::string path = "./output/" + dataset + "/degrees.txt";
    fs.open(path, std::ios_base::out);
    if (!fs.is_open())
    {
        printf("Cannot save degrees!\n");
        return;
    }
    try
    {
        for (auto d : degs)
        {
            fs << d << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        if (fs.is_open())
            fs.close();
    }
    if (fs.is_open())
        fs.close();
}

void DPGS::initLogger()
{
    const std::string path = "./output/" + dataset + "/summarize.log";
    fs.open(path, std::ios_base::out);
    if (!fs.is_open())
        printf("Fail to open log file!\n");
}

double DPGS::run(const int T, const float nodeRatio)
{
    const double slope = double(maxB - minB) / (30 - 1);

    auto start = chrono::high_resolution_clock::now();
    // auto startTime = clock(); // Use CPU time instead of wall clock time
    for (int t = 0; t < T; t++)
    {
        int b = slope * t + minB;
        if (b > maxB)
            b = maxB;
        int r = numPerm / b;
        snprintf(buffer, 128, "Iteration %d, r: %d, b: %d\n", t + 1, r, b);
        fs << buffer;
        updateLSH(r, b);
        size_t mergeCnt = 0;
        for (auto &group : groups)
            mergeCnt += mergeGroup(group);
        if (mergeCnt == 0)
        {
            printf("No merge in iteration %d.\n", t + 1);
            snprintf(buffer, 128, "No merge in iteration %d.\n", t + 1);
            fs << buffer;
            // break;
        }
        else
        {
            printf("Merge %lu in iteration %d.\n", mergeCnt, t + 1);
            snprintf(buffer, 128, "Merge %lu in iteration %d.\n", mergeCnt, t + 1);
            fs << buffer;
            if ((t + 1) % 5 == 0)
            {
                snprintf(buffer, 128, "Current gain: %.2f\n", totalGain);
                fs << buffer;
            }
        }
    }
    // auto endTime = clock();
    // auto elapsed = 1000.0 * (endTime - startTime) / CLOCKS_PER_SEC;
    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration<double, std::milli>(end - start);

    printf("------------------------\n");
    printf("Summarization completes.\n");
    printf("Elapsed: %.2f ms\n", elapsed.count());
    printf("|V_s| = %lu, |E_s| = %lu\n", numNode, numEdge);
    fs << "------------------------\n";
    fs << "Summarization completes.\n";
    snprintf(buffer, 128, "Elapsed: %.2f ms\n", elapsed.count());
    fs << buffer;
    snprintf(buffer, 128, "|V_s| = %lu, |E_s| = %lu\n", numNode, numEdge);
    fs << buffer;

    double modelLen = modelLength();
    initKLError = finalKLError();
    double compRatio = (modelLen + initKLError) / origLen;

    printf("Original bits: %.4f\n", origLen);
    printf("Model bits: %.4f\n", modelLen);
    printf("KL divergence: %.4e\n", initKLError / (N*N));
    printf("Relative size of output: %.4f\n", modelLen / origLen);
    printf("Compression ratio: %.4f\n", compRatio);
    snprintf(buffer, 128, "Original bits: %.4f\n", origLen);
    fs << buffer;
    snprintf(buffer, 128, "Model bits: %.4f\n", modelLen);
    fs << buffer;
    snprintf(buffer, 128, "KL divergence: %.4e\n", initKLError / (N*N));
    fs << buffer;
    snprintf(buffer, 128, "Relative size of output: %.4f\n", modelLen / origLen);
    fs << buffer;
    snprintf(buffer, 128, "Compression ratio: %.4f\n", compRatio);
    fs << buffer;

    return 0;
}

double DPGS::modelLength()
{
    double ret = 0.0;
    ret += LN(numNode);
    ret += N * LN(numNode);
    ret += degPartLength;
    ret += LN(numEdge);
    ret += LnU(numNode * (numNode + 1) / 2, numEdge);
    for (int64_t n = 0; n < N; n++)
    {
        if (isDeleted(n))
            continue;
        auto &neis = graph.Neighbors(n);
        for (auto &p : neis)
        {
            if (isDeleted(p.first))
                continue;
            if (n <= p.first)
                ret += LN(p.second);
        }
    }

    return ret;
}

double DPGS::finalKLError()
{
    double ret = initKLError;
    for (int64_t n = 0; n < N; n++)
    {
        if (isDeleted(n))
            continue;
        auto &neis = graph.Neighbors(n);
        int64_t d = 0;
        for (auto &p : neis)
        {
            if (isDeleted(p.first))
                continue;
            ret -= xlogx(p.second);
            d += p.second;
        }
        ret += 2 * xlogx(d);
    }

    return ret;
}

void DPGS::parallelUpdateLSH(const int64_t start, const int64_t end)
{
    static auto getFirst =
        [](const std::pair<int64_t, int> &p) {
            return p.first;
        };
    for (int64_t u = start; u < end; u++)
    {
        if (isDeleted(u))
            continue;
        auto &&adjList = graph.Neighbors(u);
        std::vector<int64_t> neis{u};
        std::transform(adjList.begin(), adjList.end(),
                       std::back_inserter(neis), getFirst);
        auto hashValues = minhash.HashItems(neis.begin(), neis.end());
        lsh.insert(u, hashValues);
    }
}

void DPGS::updateLSH(const int r, const int b)
{
    minhash.InitPerm();
    lsh.setParam(r, b);
    // Update LSH of each nodes
    // auto threadNum = std::thread::hardware_concurrency();
    auto threadNum = 8;
    std::vector<std::thread> threads(threadNum - 1);
    const int64_t gap = N / threadNum;
    int64_t idx = 0;
    for (int i = 0; i < threadNum - 1; i++)
    {
        threads[i] = std::thread(&DPGS::parallelUpdateLSH, this, idx, idx + gap);
        idx += gap;
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    parallelUpdateLSH(idx, N);

    // Group nodes in same LSH buckets
    UnionFind uf(N);
    auto &hashtables = lsh.GetHashTable();
    for (auto &table : hashtables)
    {
        for (auto &p : table)
        {
            auto &nodes = p.second;
            if (nodes.size() <= 1)
                continue;
            int64_t first_node = *nodes.begin();
            for (auto node : nodes)
            {
                if (node == first_node)
                    continue;
                uf.union_(first_node, node);
            }
        }
    }
    std::unordered_map<int64_t, std::vector<int64_t>> group_map;
    for (int64_t u = 0; u < N; u++)
    {
        int64_t id = uf.root(u);
        if (uf.getSize(id) <= 1)
            continue;
        group_map[id].push_back(u);
    }
    groups.clear();
    for (auto &p : group_map)
    {
        auto iter = p.second.begin();
        auto size = p.second.size();
        while (size > C)
        {
            groups.emplace_back(std::vector<int64_t>(iter, iter + C));
            std::advance(iter, C);
            size -= C;
        }
        if (size > 0)
        {
            groups.emplace_back(std::vector<int64_t>(iter, p.second.end()));
        }
    }
}

size_t DPGS::mergeGroup(std::vector<int64_t> &group)
{
    size_t mergeCnt = 0;
    // Sample and merge
    int times = log2(group.size());
    int nSkip = 0;
    std::random_device rd;

    bool end = false;
    while (!end)
    {
        if (group.size() <= 1)
            break;
        double maxGain = 0;
        int64_t maxNewE = -1;
        int64_t maxU, maxV;

        std::vector<size_t> indices(group.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::default_random_engine engine(rd());
        std::shuffle(indices.begin(), indices.end(), engine);
        size_t posV = -1;
        for (int i = 0; i < times; i++)
        {
            if (group.size() <= 1)
                break;

            size_t idx1 = i % group.size(), idx2 = (i + 1) % group.size();
            idx1 = indices[idx1], idx2 = indices[idx2];
            int64_t u = group[idx1], v = group[idx2];

            auto pair = mergeGain(u, v);
            double gain = pair.first;
            int64_t new_e = pair.second;
            if (std::isnan(gain))
            {
                printf("Get unexpected gain NaN!\n");
                break;
            }
            if (gain > maxGain)
            {
                maxGain = gain;
                maxU = u;
                maxV = v;
                posV = idx2;
                maxNewE = new_e;
            }
        }
        if (maxGain > 0)
        {
            nSkip = 0;
            merge(maxU, maxV);
            mergeCnt += 1;
            numEdge = maxNewE;
            numNode--;
            totalGain += maxGain;
            group.erase(group.begin() + posV);

            if (debug)
            {
                snprintf(buffer, 128, "Merge (%lu, %lu), gain: %.2f\n", maxU, maxV, maxGain);
                fs << buffer;
            }
        }
        else
        {
            nSkip += 1;
            if (nSkip >= times)
                end = true;
        }
    }
    return mergeCnt;
}

std::pair<double, int64_t> DPGS::mergeGain(const int64_t u, const int64_t v)
{
    double gain = LN(numNode) - LN(numNode - 1);
    auto du = degs[u], dv = degs[v];
    gain += 2 * (xlogx(du) + xlogx(dv) - xlogx(du + dv));
    // gain += N * log2(double(numNode) / (numNode - 1));
    gain += N * (LN(numNode) - LN(numNode - 1));

    // Computing gain from common neighbors
    auto &nei_u = graph.Neighbors(u);
    auto &nei_v = graph.Neighbors(v);
    std::vector<int64_t> common_neis;
    std::for_each(nei_u.begin(), nei_u.end(),
                  [&nei_v, &common_neis](const std::pair<int64_t, int> &pair) {
                      auto iter = nei_v.find(pair.first);
                      if (iter != nei_v.end())
                      {
                          common_neis.push_back(pair.first);
                      }
                  });
    int64_t n_common_nei = 0;
    for (auto nei : common_neis)
    {
        if (nei == u || nei == v || isDeleted(nei))
            continue;
        n_common_nei++;
        auto un = nei_u[nei], vn = nei_v[nei];
        auto newWeight = un + vn;
        gain += 2 * xlogx(newWeight);
        gain -= 2 * (xlogx(un) + xlogx(vn));
        gain += LN(un) + LN(vn);
        gain -= LN(newWeight);
    }
    int64_t newNumEdge = numEdge - n_common_nei;

    // Dealing with self-loops
    bool uinu = nei_u.find(u) != nei_u.end();
    bool uinv = nei_u.find(v) != nei_u.end();
    bool vinv = nei_v.find(v) != nei_v.end();
    if (uinu || uinv || vinv)
    {
        int uu = 0, uv = 0, vv = 0;
        if (uinu)
        {
            uu = nei_u[u];
            gain += LN(uu);
            gain -= xlogx(uu);
        }
        if (uinv)
        {
            uv = nei_u[v];
            gain += LN(uv);
            gain -= 2 * xlogx(uv);
        }
        if (vinv)
        {
            vv = nei_v[v];
            gain += LN(vv);
            gain -= xlogx(vv);
        }
        int new_weight = uu + 2 * uv + vv;
        gain -= LN(new_weight);
        gain += xlogx(new_weight);

        newNumEdge -= (uinu + uinv + vinv - 1);
    }
    else
    {
        if (newNumEdge == numEdge)
            return {0, newNumEdge};
    }

    gain += LN(numEdge) - LN(newNumEdge);
    gain += LnU(numNode * (numNode + 1) / 2, numEdge);
    gain -= LnU(numNode * (numNode - 1) / 2, newNumEdge);
    return {gain, newNumEdge};
}

void DPGS::merge(const int64_t u, const int64_t v)
{
    std::move(nodesDict[v].begin(), nodesDict[v].end(), std::back_inserter(nodesDict[u]));
    nodesDict[v].clear();

    degs[u] += degs[v];
    degs[v] = 0;

    for (auto &p : graph.Neighbors(v))
    {
        if (p.first == u || p.first == v || isDeleted(p.first))
            continue;
        graph.deltaEdge(u, p.first, p.second);
    }
    int uv = graph.edge(u, v), vv = graph.edge(v, v);
    graph.deltaEdge(u, u, 2 * uv + vv);
    graph.deleteEdge(u, v);
    graph.deleteNode(v);

    for (auto &p : graph.Neighbors(u))
    {
        int64_t nei = p.first;
        if (nei != u && nei != v && !isDeleted(nei))
        {
            if (p.second != 0)
                graph.setEdge(p.first, u, p.second);
        }
    }
    // deletedNodes.insert(v);
}

bool DPGS::isDeleted(const int64_t u)
{
    return graph.Neighbors(u).empty();
}

void DPGS::saveResult(const std::string &dataset)
{
    std::stringstream ss;
    ss << "./output/" << dataset;
    const std::string dir(ss.str());
    const std::string path = dir + "/summary.edgelist";
    std::ofstream fs(path, std::ios_base::out);
    if (!fs.is_open())
    {
        printf("Cannot open file %s!\n", path.c_str());
        return;
    }

    try
    {
        fs << "# Number of nodes: " << numNode;

        const auto &adjLists = graph.adjaceny();
        for (int64_t u = 0; u < N; u++)
        {
            if (isDeleted(u))
                continue;
            for (auto &p : adjLists[u])
            {
                if (p.first >= u && !isDeleted(p.first))
                    fs << '\n'
                       << u << '\t' << p.first << '\t' << p.second;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        if (fs.is_open())
            fs.close();
    }
    if (fs.is_open())
        fs.close();

    // Save supernodes information
    fs.open(dir + "/supernodes.txt", std::ios_base::out);
    // fs.open("./output/supernodes.txt", std::ios_base::out);
    if (!fs.is_open())
    {
        printf("Cannot save supernode dict!\n");
        return;
    }
    try
    {
        for (int64_t u = 0; u < N; u++)
        {
            if (nodesDict[u].empty())
                continue;
            fs << u;
            for (int64_t n : nodesDict[u])
            {
                fs << "\t" << n;
            }
            fs << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        if (fs.is_open())
            fs.close();
    }
    if (fs.is_open())
        fs.close();
}

void DPGS::printInfo()
{
    printf("------------------------\n");
    fs << "------------------------\n";
    printf("|V_s| = %lu, |E_s| = %lu\n", numNode, numEdge);
    snprintf(buffer, 128, "Current frac: %.2f\n", currentFrac);
    fs << buffer;
    snprintf(buffer, 128, "|V_s| = %lu, |E_s| = %lu\n", numNode, numEdge);
    fs << buffer;

    double modelLen = modelLength();
    double KLError = finalKLError();
    double compRatio = (modelLen + KLError) / origLen;

    printf("Original bits: %.4f\n", origLen);
    printf("Model bits: %.4f\n", modelLen);
    printf("KL divergence: %.4e\n", KLError / (N*N));
    printf("Relative size of output: %.4f\n", modelLen / origLen);
    printf("Compression ratio: %.4f\n", compRatio);
    snprintf(buffer, 128, "Original bits: %.4f\n", origLen);
    fs << buffer;
    snprintf(buffer, 128, "Model bits: %.4f\n", modelLen);
    fs << buffer;
    snprintf(buffer, 128, "KL divergence: %.4e\n", KLError / (N*N));
    fs << buffer;
    snprintf(buffer, 128, "Relative size of output: %.4f\n", modelLen / origLen);
    fs << buffer;
    snprintf(buffer, 128, "Compression ratio: %.4f\n", compRatio);
    fs << buffer;
}