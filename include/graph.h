#ifndef __SUMM_GRAPH_H_
#define __SUMM_GRAPH_H_

#include <algorithm>
#include <vector>
#include <unordered_map>

class Graph
{
public:
    using adjListT = std::unordered_map<int64_t, int>;

    Graph() = default;
    Graph(const int64_t n);
    int64_t numNode() const { return n; }
    int64_t numEdge()
    {
        if (m == 0)
        {
            std::for_each(adjLists.begin(), adjLists.end(),
                          [this](const adjListT &adjlist) {
                              m += adjlist.size();
                          });
            m /= 2;
        }
        return m;
    }
    void setNumNode(const int64_t n) { this->n = n; }
    void deleteNode(const int64_t u) { adjLists[u].clear(); }

    int edge(const int64_t u, const int64_t v) const;
    void deltaEdge(const int64_t u, const int64_t v, const int weight = 1);
    void setEdge(const int64_t u, const int64_t v, const int weight = 1);
    void deleteEdge(const int64_t u, const int64_t v) { adjLists[u].erase(v); }

    adjListT &Neighbors(const int64_t u);
    const std::vector<adjListT>& adjaceny() const { return adjLists; }

private:
    int64_t n;
    int64_t m{0};
    std::vector<adjListT> adjLists;
};

#endif