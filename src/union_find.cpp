#include "union_find.h"
#include <numeric>
#include <cstdio>

UnionFind::UnionFind(const int64_t n)
    : N(n)
{
    ids.resize(N);
    std::iota(ids.begin(), ids.end(), 0);
    sizes.resize(N);
    std::fill(sizes.begin(), sizes.end(), 1);
}

void UnionFind::union_(const int64_t x, const int64_t y)
{
    int64_t rootx = root(x);
    int64_t rooty = root(y);
    if (rootx == rooty)
        return;
    if (sizes[rootx] >= sizes[rooty])
    {
        ids[rooty] = rootx;
        sizes[rootx] += sizes[rooty];
    }
    else
    {
        ids[rootx] = rooty;
        sizes[rooty] += sizes[rootx];
    }
}

int64_t UnionFind::find(const int64_t x, const int64_t y)
{
    return root(x) == root(y);
}

int64_t UnionFind::root(const int64_t x)
{
    if (ids[x] == x)
        return x;
    ids[x] = root(ids[x]);
    return ids[x];
}