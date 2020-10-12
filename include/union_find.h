#ifndef __SUMM_UNION_FIND_H_
#define __SUMM_UNION_FIND_H_

#include <cstdint>
#include <cstdlib>
#include <vector>

class UnionFind
{
public:
    UnionFind(const int64_t n);
    void union_(const int64_t x, const int64_t y);
    int64_t find(const int64_t x, const int64_t y);
    int64_t root(const int64_t x);
    size_t getSize(const int64_t x) const { return sizes[x]; }
private:
    int64_t N;
    std::vector<int64_t> ids;
    std::vector<size_t> sizes;
};

#endif
