#ifndef __MATRIX1_H__
#define __MATRIX1_H__
#include <cassert>
#include <functional>
#include <iostream>

using namespace std;

/// Imprime un elemento seguido de espacio. Para usar con `ApplyFunctionToAll`.
template <typename T>
void PrintM(T &n, ostream &os) { os << n << " "; }

/**
 * # Matrix1\<T\>
 * Matriz 2D dinámica en heap. Análogo a `Array4` para estructuras bidimensionales.
 *
 * ## Almacenamiento interno
 * ```
 * m_pMat → [ fila0* | fila1* | ... ]
 *                ↓        ↓
 *            [e00 e01] [e10 e11] ...
 * ```
 *
 * ## Formato de stream
 * Entrada/salida: `rows cols e00 e01 ... e(R-1)(C-1)`
 */
template <typename T>
class Matrix1 {
    private:
        T      **m_pMat = nullptr;  ///< arreglo de punteros a filas
        size_t   m_rows = 0;        ///< número de filas
        size_t   m_cols = 0;        ///< número de columnas
    public:
        Matrix1()  { }
        ~Matrix1() { Destroy(); }

        /// Aloca memoria. Requiere `m_rows > 0` y `m_cols > 0` (asignados por `Read`).
        void     Create();

        /// Lee `rows cols` del stream, llama `Create()`, luego lee elementos fila por fila.
        /// @return referencia al stream para encadenamiento
        istream &Read(istream &is);

        /**
         * Aplica `func(m_pMat[i][j], args...)` a cada elemento.
         *
         * Ejemplos:
         * ```cpp
         * mat.ApplyFunctionToAll(Square<int>);          // n *= n
         * mat.ApplyFunctionToAll(AddX<int>, 5, 10);     // n += 15
         * mat.ApplyFunctionToAll(PrintM<int>, cout);    // imprime cada elemento
         * ```
         */
        template <typename Func, typename... Args>
        void ApplyFunctionToAll(Func func, Args&& ...args);

        /// Imprime `rows cols \n` seguido de cada fila en su propia línea.
        /// @return referencia al stream para encadenamiento
        ostream &Print(ostream &os);

        /// Libera memoria fila por fila, luego el arreglo de punteros. Pone `m_pMat = nullptr`.
        void Destroy();
};

template <typename T>
void Matrix1<T>::Create() {
    assert(m_rows > 0 && m_cols > 0);
    m_pMat = new T *[m_rows];
    for (size_t i = 0; i < m_rows; ++i)
        m_pMat[i] = new T[m_cols];
}

template <typename T>
istream &Matrix1<T>::Read(istream &is) {
    Destroy();
    is >> m_rows >> m_cols;
    Create();
    for (size_t i = 0; i < m_rows; ++i)
        for (size_t j = 0; j < m_cols; ++j)
            is >> m_pMat[i][j];
    return is;
}

template <typename T>
template <typename Func, typename... Args>
void Matrix1<T>::ApplyFunctionToAll(Func func, Args&& ...args) {
    for (size_t i = 0; i < m_rows; ++i)
        for (size_t j = 0; j < m_cols; ++j)
            func(m_pMat[i][j], forward<Args>(args)...);
}

template <typename T>
ostream &Matrix1<T>::Print(ostream &os) {
    os << m_rows << " " << m_cols << " \n";
    for (size_t i = 0; i < m_rows; ++i) {
        for (size_t j = 0; j < m_cols; ++j)
            os << m_pMat[i][j] << " ";
        os << "\n";
    }
    return os;
}

template <typename T>
void Matrix1<T>::Destroy() {
    if (m_pMat != nullptr) {
        for (size_t i = 0; i < m_rows; ++i)
            delete[] m_pMat[i];
        delete[] m_pMat;
        m_pMat = nullptr;
    }
}

/// `is >> mat` delega a `mat.Read(is)`
template <typename T>
istream &operator>>(istream &is, Matrix1<T> &mat) {
    return mat.Read(is);
}

/// `os << mat` delega a `mat.Print(os)`
template <typename T>
ostream &operator<<(ostream &os, Matrix1<T> &mat) {
    return mat.Print(os);
}

#endif // __MATRIX1_H__
