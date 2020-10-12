#include "graph.h"

Graph::Graph(const int64_t n)
    : n(n)
{
    adjLists.resize(n);
}

int Graph::edge(const int64_t u, const int64_t v) const
{
    auto &adjlist = adjLists[u];
    auto it = adjlist.find(v);
    if (it == adjlist.end())
        return 0;
    return it->second;
}

void Graph::deltaEdge(const int64_t u, const int64_t v, const int weight)
{
    if (weight == 0)
        return;
    auto iter = adjLists[u].find(v);
    if (iter == adjLists[u].end())
        adjLists[u].insert({v, weight});
    else
        iter->second += weight;
}

void Graph::setEdge(const int64_t u, const int64_t v, const int weight)
{
    if (weight == 0)
        return;
    auto iter = adjLists[u].find(v);
    if (iter == adjLists[u].end())
        adjLists[u].insert({v, weight});
    else
        iter->second = weight;
}

Graph::adjListT &Graph::Neighbors(const int64_t u)
{
    return adjLists[u];
}