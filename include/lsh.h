#ifndef __SUMM_LSH_H_
#define __SUMM_LSH_H_

#include <algorithm>
#include <bitset>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "minhash.h"

template <int numPerm, class Key = int>
class LSH
{
    using Signature = std::bitset<64 * numPerm>;
    using Sets = std::unordered_set<Key>;
    using HashTable = std::unordered_map<Signature, Sets>;

public:
    LSH() = default;
    LSH(const int r, const int b);

    void insert(const Key &key, const std::vector<int64_t> &hashValues);

    void setParam(const int r, const int b)
    {
        this->r = r;
        this->b = b;
        hashtables.clear();
        hashtables.resize(b);
    }

    std::vector<HashTable> &GetHashTable()
    {
        return hashtables;
    }

private:
    int r;
    int b;
    std::mutex m;
    std::vector<HashTable> hashtables;
};

template <int numPerm, class Key>
LSH<numPerm, Key>::LSH(const int r, const int b)
    : r(r), b(b)
{
    hashtables.resize(b);
}

template <int numPerm, class Key>
void LSH<numPerm, Key>::insert(const Key &key, const std::vector<int64_t> &hashValues)
{
    for (int i = 0; i < b; i++)
    {
        Signature sign(0);
        for (size_t j = i * r; j < (i + 1) * r; j++)
        {
            if (j >= hashValues.size())
                break;
            sign <<= 64;
            sign |= hashValues[j];
        }
        {
            std::lock_guard<std::mutex> lk(m);
            hashtables[i][sign].insert(key);
        }
    }
}


#endif
