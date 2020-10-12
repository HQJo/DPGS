#ifndef __SUMM_UTILS_H_
#define __SUMM_UTILS_H_

#include <fstream>
#include "graph.h"

Graph readEdgelist(const std::string &path, const char comment = '#',
                   const char delimeter = '\t', bool symm = true);

#endif