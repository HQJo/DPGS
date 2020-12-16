#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "graph.h"
#include "summarizer.h"

namespace py = pybind11;

Graph fromRowCol(uint64_t N, py::array_t<uint64_t> rows, py::array_t<uint64_t> cols)
{
    Graph graph(N);
    uint64_t *ptr1 = static_cast<uint64_t *>(rows.request().ptr);
    uint64_t *ptr2 = static_cast<uint64_t *>(cols.request().ptr);
    size_t size = rows.request().size;

    for (size_t i = 0; i < size; i++)
    {
        graph.setEdge(ptr1[i], ptr2[i]);
    }

    return graph;
}

PYBIND11_MODULE(DPGS, m)
{
    m.def("from_row_col", &fromRowCol);
    py::class_<Graph>(m, "Graph");

    py::class_<DPGS>(m, "DPGS")
        .def(py::init<const std::string &, const Graph &, const int, const int, bool>())
        .def("run", &DPGS::run)
        .def("getNodesDict", &DPGS::getNodesDict);
}