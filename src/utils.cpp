#include "utils.h"
#include <iostream>

Graph readEdgelist(const std::string &path, const char comment,
                   const char delimeter, bool symm)
{
    std::ifstream fs(path, std::ios_base::in);
    if (!fs.is_open())
    {
        throw std::ios_base::failure("Can't open file: " + path);
    }

    std::string line;
    int64_t N;
    fs >> N;
    Graph graph(N);
    try
    {
        while (!fs.eof())
        {
            getline(fs, line);
            if (line.size() <= 1)
                continue;
            if (line[0] == comment)
                continue;
            size_t pos = line.find(delimeter);
            int64_t u = std::stoull(line.substr(0, pos));
            int64_t v = std::stoull(line.substr(pos + 1));
            if (u == v)
                continue;
            else
                graph.setEdge(u, v);
            if (symm && u != v)
                graph.setEdge(v, u);
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

    return graph;
}
