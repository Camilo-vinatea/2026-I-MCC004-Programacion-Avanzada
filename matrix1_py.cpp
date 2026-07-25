/**
 * @file matrix1_py.cpp
 * @brief Bindings pybind11 de Matrix1\<T\> para Python.
 * @ingroup matrix
 *
 * @details
 * Expone `Matrix1<int>` y `Matrix1<double>` como clases Python `MatrixI` / `MatrixD`
 * en el módulo `matrix1`. Incluye el proxy de fila `RowProxy<T>` que habilita
 * el acceso `m[i][j]` desde Python usando índices consecutivos.
 *
 * ### Clases exportadas al módulo `matrix1`
 * | Nombre Python | Tipo C++          | Tipo elemento |
 * |---------------|-------------------|---------------|
 * | `MatrixI`     | `Matrix1<int>`    | `int`         |
 * | `MatrixD`     | `Matrix1<double>` | `double`      |
 * | `RowProxyI`   | `RowProxy<int>`   | `int`         |
 * | `RowProxyD`   | `RowProxy<double>`| `double`      |
 *
 * ### Métodos Python disponibles
 * ```python
 * m = MatrixI()
 * m.from_string("2 3  1 2 3  4 5 6")   # leer desde string
 * print(m.rows, m.cols)                  # dimensiones
 * print(m)                               # __str__
 * r = m + n; r = m - n; r = m * n       # __add__, __sub__, __mul__
 * r = m * 5; r = 5 * m                  # __mul__, __rmul__
 * v = m[i][j]; m[i][j] = v              # __getitem__ + RowProxy
 * ```
 *
 * ### Patrón de punteros a método (dispatch)
 * Para cubrir el requerimiento académico de usar `.*` y `->*`:
 * @code{.cpp}
 * // dispatch via .*  (objeto)
 * return (lhs.*method)(rhs);
 *
 * // dispatch via ->*  (puntero)
 * return (pLhs->*method)(rhs);
 * @endcode
 */

#include <pybind11/pybind11.h>
#include <sstream>
#include "matrix1.h"
#include "types.h"

namespace py = pybind11;

// ---------------------------------------------------------------------------
// Alias de tipo para punteros a método miembro
// ---------------------------------------------------------------------------

/**
 * @brief Puntero a método binario matriz×matriz de Matrix1\<T\>.
 * @tparam T Tipo de elemento.
 * @details Cubre `operator+`, `operator-` y `operator*(const Matrix1&)`.
 * @ingroup matrix
 */
template <typename T>
using BinOp = Matrix1<T> (Matrix1<T>::*)(const Matrix1<T>&) const;

/**
 * @brief Puntero a método de multiplicación por escalar de Matrix1\<T\>.
 * @tparam T Tipo de elemento.
 * @details Cubre `operator*(T value)`.
 * @ingroup matrix
 */
template <typename T>
using ScalarOp = Matrix1<T> (Matrix1<T>::*)(T) const;

// ---------------------------------------------------------------------------
// Funciones de despacho (requerimiento académico: .* y ->*)
// ---------------------------------------------------------------------------

/**
 * @brief Invoca un método miembro binario vía el operador `.*`.
 * @tparam T Tipo de elemento de la matriz.
 * @param lhs    Operando izquierdo (referencia a objeto).
 * @param rhs    Operando derecho (const ref).
 * @param method Puntero al método a invocar.
 * @return Resultado de `(lhs.*method)(rhs)`.
 * @note Demuestra el uso del operador `.*` sobre un objeto (no puntero).
 * @ingroup matrix
 */
template <typename T>
Matrix1<T> dispatch(Matrix1<T>& lhs, const Matrix1<T>& rhs, BinOp<T> method) {
    return (lhs.*method)(rhs);
}

/**
 * @brief Invoca un método miembro binario vía el operador `->*`.
 * @tparam T Tipo de elemento de la matriz.
 * @param pLhs   Operando izquierdo (puntero a objeto).
 * @param rhs    Operando derecho (const ref).
 * @param method Puntero al método a invocar.
 * @return Resultado de `(pLhs->*method)(rhs)`.
 * @note Demuestra el uso del operador `->*` sobre un puntero.
 * @ingroup matrix
 */
template <typename T>
Matrix1<T> dispatch_ptr(Matrix1<T>* pLhs, const Matrix1<T>& rhs, BinOp<T> method) {
    return (pLhs->*method)(rhs);
}

// ---------------------------------------------------------------------------
// RowProxy<T>
// ---------------------------------------------------------------------------

/**
 * @brief Proxy de fila que habilita el acceso `m[i][j]` en Python.
 *
 * @tparam T Tipo de elemento de la matriz subyacente.
 *
 * @details
 * En Python, `m[i]` llama a `Matrix1.__getitem__(i)` y devuelve un `RowProxy`.
 * Sobre ese proxy, `[j]` llama a `RowProxy.__getitem__(j)` para lectura o
 * `RowProxy.__setitem__(j, val)` para escritura.
 *
 * Esto permite la sintaxis natural:
 * @code{.python}
 * val = m[1][2]    # lectura
 * m[0][3] = 99     # escritura
 * @endcode
 *
 * @note El proxy guarda una referencia a la matriz original; el ciclo de vida
 *       del proxy debe ser menor que el de la matriz.
 *
 * ### Relación con Matrix1
 * @dot
 * digraph rowproxy {
 *   rankdir=LR;
 *   node [shape=record, style=filled, fontname="Helvetica", fontsize=10];
 *   M [label="{Matrix1\<T\>|operator[](size_t) → T*}", fillcolor="#aaccff"];
 *   R [label="{RowProxy\<T\>|m_matrix : Matrix1\<T\>\&|m_row : size_t|operator[](col) → T\&}", fillcolor="#ccffcc"];
 *   M -> R [label=" __getitem__(i)"];
 *   R -> M [label=" delega m_matrix[m_row][col]", style=dashed];
 * }
 * @enddot
 *
 * @ingroup matrix
 */
template <typename T>
class RowProxy {
    Matrix1<T>& m_matrix;  ///< Referencia a la matriz contenedora.
    size_t       m_row;    ///< Índice de la fila representada.
public:
    /**
     * @brief Construye un proxy para la fila `row` de `matrix`.
     * @param matrix Matriz subyacente (referencia no-const).
     * @param row    Índice de la fila (0-based).
     */
    RowProxy(Matrix1<T>& matrix, size_t row) : m_matrix(matrix), m_row(row) {}

    /**
     * @brief Acceso mutable al elemento en columna `col`.
     * @param col Índice de columna (0-based).
     * @return Referencia al elemento `m_matrix[m_row][col]`.
     */
    T&       operator[](size_t col)       { return m_matrix[m_row][col]; }

    /**
     * @brief Acceso de solo lectura al elemento en columna `col`.
     * @param col Índice de columna (0-based).
     * @return Referencia const al elemento `m_matrix[m_row][col]`.
     */
    const T& operator[](size_t col) const { return m_matrix[m_row][col]; }
};

// ---------------------------------------------------------------------------
// Función de registro pybind11
// ---------------------------------------------------------------------------

/**
 * @brief Registra Matrix1\<T\> y RowProxy\<T\> como clases Python en el módulo.
 *
 * @tparam T           Tipo de elemento (e.g., `int`, `double`).
 * @param m            Módulo pybind11 destino.
 * @param matrixName   Nombre de la clase Python para Matrix1\<T\> (e.g., `"MatrixI"`).
 * @param rowProxyName Nombre de la clase Python para RowProxy\<T\> (e.g., `"RowProxyI"`).
 *
 * @details
 * Métodos Python registrados sobre la clase `matrixName`:
 * | Método Python   | Equivalente C++                        |
 * |-----------------|----------------------------------------|
 * | `from_string(s)`| `Read(istringstream(s))`               |
 * | `.rows`         | `rows()` (property)                    |
 * | `.cols`         | `cols()` (property)                    |
 * | `__str__`       | `Print(ostringstream)`                 |
 * | `__add__`       | `dispatch(lhs, rhs, &M::operator+)`    |
 * | `__sub__`       | `dispatch(lhs, rhs, &M::operator-)`    |
 * | `__mul__` (mat) | `dispatch_ptr(pLhs, rhs, &M::operator*)` |
 * | `__mul__` (esc) | `(lhs.*ScalarOp)(scalar)`              |
 * | `__rmul__`      | `(rhs.*ScalarOp)(scalar)`              |
 * | `__getitem__`   | Devuelve `RowProxy<T>(self, row)`      |
 *
 * @ingroup matrix
 */
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

// ---------------------------------------------------------------------------
// Punto de entrada del módulo Python
// ---------------------------------------------------------------------------

/**
 * @brief Punto de entrada del módulo pybind11 `matrix1`.
 * @details Registra:
 * - `MatrixI` / `RowProxyI` para `Matrix1<int>`
 * - `MatrixD` / `RowProxyD` para `Matrix1<double>`
 */
PYBIND11_MODULE(matrix1, m) {
    bind_matrix<TI>(m, "MatrixI", "RowProxyI");  ///< Matriz de enteros.
    bind_matrix<TD>(m, "MatrixD", "RowProxyD");  ///< Matriz de double.
}
