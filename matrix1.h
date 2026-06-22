#ifndef __MATRIX1_H__
#define __MATRIX1_H__
#include <cassert>
#include <functional>
#include <iostream>
#include <utility>

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

        Matrix1(const Matrix1 &other) = delete;
        Matrix1(Matrix1 &&other) noexcept;
        Matrix1 &operator=(const Matrix1 &other) = delete;
        Matrix1 &operator=(Matrix1 &&other) noexcept;

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
        ostream &Print(ostream &os) const;

        /// Libera memoria fila por fila, luego el arreglo de punteros. Pone m_pMat = nullptr.
        void Destroy();

        /// Suma elemento a elemento.
        Matrix1 operator+(const Matrix1 &other) const;

        /// Resta elemento a elemento.
        Matrix1 operator-(const Matrix1 &other) const;

        /// Multiplicación matricial.
        Matrix1 operator*(const Matrix1 &other) const;

        /// Multiplicación por escalar.
        Matrix1 operator*(T value) const;

        size_t rows() const { return m_rows; }
        size_t cols() const { return m_cols; }

        /// Acceso a elemento (i,j). Primer [] retorna puntero a fila i; segundo [] accede columna j.
        T *operator[](size_t i);
        const T *operator[](size_t i) const;
};

template <typename T>
void Matrix1<T>::Create() {
    assert(m_rows > 0 && m_cols > 0);
    m_pMat = new T *[m_rows];
    for (size_t i = 0; i < m_rows; ++i)
        m_pMat[i] = new T[m_cols];
}

// Move constructor
template <typename T>
Matrix1<T>::Matrix1(Matrix1 &&other) noexcept {
    m_pMat = exchange(other.m_pMat, nullptr);
    m_rows = exchange(other.m_rows, 0);
    m_cols = exchange(other.m_cols, 0);
}

// Move assignment
template <typename T>
Matrix1<T> &Matrix1<T>::operator=(Matrix1 &&other) noexcept {
    if (this != &other) {
        Destroy();
        m_pMat = exchange(other.m_pMat, nullptr);
        m_rows = exchange(other.m_rows, 0);
        m_cols = exchange(other.m_cols, 0);
    }
    return *this;
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
ostream &Matrix1<T>::Print(ostream &os) const {
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

template <typename T>
Matrix1<T> Matrix1<T>::operator+(const Matrix1 &other) const {
    assert(m_rows == other.m_rows && m_cols == other.m_cols);
    Matrix1 result;
    result.m_rows = m_rows;
    result.m_cols = m_cols;
    result.Create();
    for (size_t i = 0; i < m_rows; ++i)
        for (size_t j = 0; j < m_cols; ++j)
            result.m_pMat[i][j] = m_pMat[i][j] + other.m_pMat[i][j];
    return result;
}

template <typename T>
Matrix1<T> Matrix1<T>::operator-(const Matrix1 &other) const {
    assert(m_rows == other.m_rows && m_cols == other.m_cols);
    Matrix1 result;
    result.m_rows = m_rows;
    result.m_cols = m_cols;
    result.Create();
    for (size_t i = 0; i < m_rows; ++i)
        for (size_t j = 0; j < m_cols; ++j)
            result.m_pMat[i][j] = m_pMat[i][j] - other.m_pMat[i][j];
    return result;
}

template <typename T>
Matrix1<T> Matrix1<T>::operator*(const Matrix1 &other) const {
    assert(m_cols == other.m_rows);
    Matrix1 result;
    result.m_rows = m_rows;
    result.m_cols = other.m_cols;
    result.Create();
    for (size_t i = 0; i < m_rows; ++i)
        for (size_t j = 0; j < other.m_cols; ++j) {
            result.m_pMat[i][j] = T{};
            for (size_t k = 0; k < m_cols; ++k)
                result.m_pMat[i][j] += m_pMat[i][k] * other.m_pMat[k][j];
        }
    return result;
}

template <typename T>
Matrix1<T> Matrix1<T>::operator*(T value) const {
    Matrix1 result;
    result.m_rows = m_rows;
    result.m_cols = m_cols;
    result.Create();
    for (size_t i = 0; i < m_rows; ++i)
        for (size_t j = 0; j < m_cols; ++j)
            result.m_pMat[i][j] = m_pMat[i][j] * value;
    return result;
}

template <typename T>
T *Matrix1<T>::operator[](size_t i) {
    assert(i < m_rows);
    return m_pMat[i];
}

template <typename T>
const T *Matrix1<T>::operator[](size_t i) const {
    assert(i < m_rows);
    return m_pMat[i];
}

/// is >> mat delega a mat.Read(is)
template <typename T>
istream &operator>>(istream &is, Matrix1<T> &mat) {
    return mat.Read(is);
}

/// os << mat delega a mat.Print(os)
template <typename T>
ostream &operator<<(ostream &os, const Matrix1<T> &mat) {
    return mat.Print(os);
}

/// Escalar * matriz: 5 * mat delega a mat * 5
template <typename T>
Matrix1<T> operator*(T value, const Matrix1<T> &mat) {
    return mat * value;
}

#endif // __MATRIX1_H__
