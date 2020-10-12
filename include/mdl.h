#ifndef __SUMM_MDL_H_
#define __SUMM_MDL_H_

#include <cstdint>
#include <cmath>

inline double xlogx(const double x)
{
    if (abs(x) < 1e-6)
        return 0;
    return x * log(x);
}

double LN(const int64_t n)
{
    if (n <= 0)
        return 0;
    double ret = log2(2.865064);
    double i = log2(n);
    while (i > 0)
    {
        ret += i;
        i = log2(i);
    }
    return ret;
}

double LnU(const int64_t n, const int64_t k)
{
    if (n == 0 || k == 0 || k == n)
        return 0;
    double x = -log2(double(k) / n);
    double y = -log2(double(n - k) / n);
    return k * x + (n - k) * y;
}

#endif