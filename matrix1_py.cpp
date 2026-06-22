#include <pybind11/pybind11.h>
#include <sstream>
#include "matrix1.h"
#include "types.h"

namespace py = pybind11;

// Puntero que guarda qué operación ejecutar
template <typename T>
using BinOp = Matrix1<T> (Matrix1<T>::*)(const Matrix1<T>&) const;

template <typename T>
using ScalarOp = Matrix1<T> (Matrix1<T>::*)(T) const;

// Dispatch via .*
template <typename T>
Matrix1<T> dispatch(Matrix1<T>& lhs, const Matrix1<T>& rhs, BinOp<T> method) {
    return (lhs.*method)(rhs);
}

// Dispatch via ->*
template <typename T>
Matrix1<T> dispatch_ptr(Matrix1<T>* pLhs, const Matrix1<T>& rhs, BinOp<T> method) {
    return (pLhs->*method)(rhs);
}

// Proxy que representa una fila de Matrix1<T>. Permite m[i][j] en Python.
template <typename T>
class RowProxy {
    Matrix1<T>& m_matrix;
    size_t       m_row;
public:
    RowProxy(Matrix1<T>& matrix, size_t row) : m_matrix(matrix), m_row(row) {}
    T&       operator[](size_t col)       { return m_matrix[m_row][col]; }
    const T& operator[](size_t col) const { return m_matrix[m_row][col]; }
};

template <typename T>
void bind_matrix(py::module_& m, const TC* matrixName, const TC* rowProxyName) {
    using M   = Matrix1<T>;
    using Row = RowProxy<T>;

    py::class_<Row>(m, rowProxyName)
        .def("__getitem__", [](Row& row, size_t col) -> T  { return row[col]; })
        .def("__setitem__", [](Row& row, size_t col, T val) { row[col] = val; });

    py::class_<M>(m, matrixName)
        .def(py::init<>())
        .def("from_string", [](M& self, const std::string& data) {
            std::istringstream ss(data);
            self.Read(ss);
        })
        .def_property_readonly("rows", &M::rows)
        .def_property_readonly("cols", &M::cols)
        .def("__str__", [](const M& self) {
            std::ostringstream ss;
            self.Print(ss);
            return ss.str();
        })
        .def("__add__", [](M& lhs, const M& rhs) {
            return dispatch<T>(lhs, rhs, &M::operator+);
        })
        .def("__sub__", [](M& lhs, const M& rhs) {
            return dispatch<T>(lhs, rhs, &M::operator-);
        })
        .def("__mul__", [](M& lhs, const M& rhs) {
            M* pLhs = &lhs;
            return dispatch_ptr<T>(pLhs, rhs, &M::operator*);
        })
        .def("__mul__", [](M& lhs, T scalar) {
            ScalarOp<T> method = &M::operator*;
            return (lhs.*method)(scalar);
        })
        .def("__rmul__", [](M& rhs, T scalar) {
            ScalarOp<T> method = &M::operator*;
            return (rhs.*method)(scalar);
        })
        .def("__getitem__", [](M& self, size_t row) {
            return Row(self, row);
        });
}

PYBIND11_MODULE(matrix1, m) {
    bind_matrix<TI>     (m, "MatrixI", "RowProxyI"); //matriz de enteros
    bind_matrix<TD>     (m, "MatrixD", "RowProxyD"); //matriz de double
}
