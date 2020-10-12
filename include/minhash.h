#ifndef __SUMM_MINHASH_H_
#define __SUMM_MINHASH_H_

#include <vector>
#include <random>
#include <cstdint>

template <class ItemType = int>
class MinHash
{
public:
    MinHash() = default;
    MinHash(const unsigned n_sign, const int seed = 0);
    void InitPerm();

    template <typename Iter>
    std::vector<int64_t> HashItems(Iter first, Iter last);

    unsigned NumSignature() const { return numSign; }
    int Seed() const { return seed; }

private:
    unsigned numSign;
    int seed;

    std::mt19937 gen;
    std::vector<int64_t> a;
    std::vector<int64_t> b;
};

const static int64_t _mersenne_prime{(static_cast<int64_t>(1) << 61) - 1};
const static int64_t _max_hash{(static_cast<int64_t>(1) << 32) - 1};
const static int64_t _hash_range{static_cast<int64_t>(1) << 32};

template <class ItemType>
MinHash<ItemType>::MinHash(const unsigned n_sign, const int seed)
    : numSign(n_sign), seed(seed)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    if (seed == 0)
        gen.seed(engine());
    else
        gen.seed(seed);

    InitPerm();
}

template <class ItemType>
template <class Iter>
std::vector<int64_t> MinHash<ItemType>::HashItems(Iter first, Iter last)
{
    std::vector<int64_t> hashValues(numSign, _max_hash);

    for (auto it = first; it != last; it++)
    {
        for (unsigned i = 0;i < numSign;i++)
        {
            auto value = (*it * a[i] + b[i]) % _mersenne_prime;
            value &= _max_hash;
            if (hashValues[i] > value)
                hashValues[i] = value;
        }
    }
    return hashValues;
}

template <class ItemType>
void MinHash<ItemType>::InitPerm()
{
    a.resize(numSign);
    b.resize(numSign);
    std::uniform_int_distribution<int64_t> disa(1, _mersenne_prime - 1);
    std::uniform_int_distribution<int64_t> disb(0, _mersenne_prime - 1);
    for (size_t i = 0; i < numSign; i++)
    {
        a[i] = disa(gen);
        b[i] = disb(gen);
    }
}

#endif