#ifndef __MATRIX1_H__
#define __MATRIX1_H__

/**
 * @file matrix1.h
 * @brief Matriz 2D dinámica en heap con soporte aritmético completo.
 * @ingroup matrix
 *
 * @details
 * Define `Matrix1<T>`: matriz genérica con almacenamiento por filas en heap
 * (`T**`). Es la extensión bidimensional de `Array4<T>`.
 *
 * ### Política de propiedad (Rule of 5)
 * | Operación      | Estado   |
 * |----------------|----------|
 * | Copy constructor | `= delete` |
 * | Copy assignment  | `= delete` |
 * | Move constructor | habilitado |
 * | Move assignment  | habilitado |
 * | Destructor       | libera todas las filas |
 *
 * ### Formato de stream
 * ```
 * rows cols e00 e01 ... e0(C-1)
 *           e10 e11 ... e1(C-1)
 *           ...
 *           e(R-1)0 ... e(R-1)(C-1)
 * ```
 */

#include <cassert>
#include <functional>
#include <iostream>
#include <utility>

using namespace std;

/**
 * @defgroup matrix Matrix1 — Módulo de Matriz 2D
 * @brief Clase Matrix1\<T\> y bindings Python para matrices 2D dinámicas.
 *
 * @details
 * Evolución natural de `Array4<T>` al plano bidimensional. El módulo cubre:
 *
 * - **Almacenamiento**: arreglo de punteros a filas en heap (`T**`)
 * - **Semántica**: movimiento habilitado, copia eliminada
 * - **Aritmética**: suma/resta elemento-a-elemento, producto matricial, escalar×matriz
 * - **I/O**: `operator>>` / `operator<<` con formato `rows cols e00 e01 ...`
 * - **Funcional**: `ApplyFunctionToAll` variádico aplica cualquier función a cada elemento
 * - **Python**: bindings pybind11 exponen `MatrixI` y `MatrixD`
 *
 * ### Estructura de memoria
 * @dot
 * digraph memoria {
 *   rankdir=LR;
 *   node [shape=record, style=filled, fillcolor="#ddeeff", fontname="Helvetica", fontsize=10];
 *   edge [fontname="Helvetica", fontsize=9];
 *   obj [label="{Matrix1\<T\>|m_rows: size_t|m_cols: size_t|m_pMat: T**}", fillcolor="#aaccff"];
 *   ptr [label="{T**\n(heap array of row ptrs)|[0]|[1]|...|[m_rows-1]}"];
 *   r0  [label="{fila 0\nT[m_cols]|e00|e01|...|e0(C-1)}"];
 *   r1  [label="{fila 1\nT[m_cols]|e10|e11|...|e1(C-1)}"];
 *   rN  [label="{fila R-1\nT[m_cols]|e(R-1)0|...|e(R-1)(C-1)}"];
 *   obj -> ptr [label=" m_pMat"];
 *   ptr -> r0  [label=" [0]"];
 *   ptr -> r1  [label=" [1]"];
 *   ptr -> rN  [label=" [R-1]"];
 * }
 * @enddot
 *
 * @{
 */

// ---------------------------------------------------------------------------
// Función auxiliar global
// ---------------------------------------------------------------------------

/**
 * @brief Imprime un elemento seguido de espacio. Usada con ApplyFunctionToAll.
 * @tparam T Tipo del elemento a imprimir (debe soportar `operator<<`).
 * @param n   Elemento a imprimir (por referencia no-const).
 * @param os  Stream de salida destino.
 * @ingroup matrix
 * @see Matrix1::ApplyFunctionToAll
 */
template <typename T>
void PrintM(T &n, ostream &os) { os << n << " "; }

// ---------------------------------------------------------------------------
// Clase Matrix1<T>
// ---------------------------------------------------------------------------

/**
 * @brief Matriz 2D dinámica en heap, genérica sobre el tipo de elemento.
 *
 * @tparam T Tipo de los elementos. Debe ser DefaultConstructible, CopyAssignable
 *           y soportar `operator+`, `operator-`, `operator*` para las operaciones
 *           aritméticas, y `operator<<` / `operator>>` para I/O por stream.
 *
 * @details
 * Implementa una matriz de `m_rows × m_cols` elementos almacenados como
 * arreglo de punteros a filas independientes en heap.
 *
 * **Ciclo de vida:**
 * 1. Constructor por defecto: objeto vacío (`m_pMat = nullptr`).
 * 2. `Read(is)`: lee `rows cols` y llama `Create()`, luego lee elementos.
 * 3. Operaciones aritméticas devuelven nuevas instancias (move semantics).
 * 4. Destructor / `Destroy()`: libera cada fila y luego el arreglo de punteros.
 *
 * **Operaciones aritméticas disponibles:**
 * @code{.cpp}
 * Matrix1<int> a, b, c;
 * cin >> a >> b >> c;
 *
 * auto sum  = a + b;       // suma elemento a elemento (requiere dims iguales)
 * auto diff = a - b;       // resta elemento a elemento
 * auto prod = a * c;       // producto matricial (a.cols == c.rows)
 * auto scal = a * 5;       // cada elemento × 5
 * auto rscl = 5 * a;       // equivalente (operator* libre)
 * @endcode
 *
 * @ingroup matrix
 */
template <typename T>
class Matrix1 {
private:
    T      **m_pMat = nullptr;  ///< Arreglo de punteros a filas (heap).
    size_t   m_rows = 0;        ///< Número de filas.
    size_t   m_cols = 0;        ///< Número de columnas.

public:

    // ------------------------------------------------------------------
    // Constructores y destructor
    // ------------------------------------------------------------------

    /**
     * @brief Constructor por defecto. Produce matriz vacía (sin memoria asignada).
     * @post `m_pMat == nullptr`, `m_rows == 0`, `m_cols == 0`.
     */
    Matrix1() { }

    /**
     * @brief Destructor. Libera toda la memoria llamando a Destroy().
     * @post Toda la memoria heap asociada queda liberada.
     */
    ~Matrix1() { Destroy(); }

    /**
     * @brief Constructor de copia eliminado (no se permite copia profunda).
     * @details Eliminado deliberadamente: Matrix1 usa semántica de movimiento.
     *          Para transferir datos use `std::move()`.
     */
    Matrix1(const Matrix1 &other) = delete;

    /**
     * @brief Constructor de movimiento. Transfiere propiedad de memoria en O(1).
     * @param other Instancia origen. Queda en estado vacío tras el movimiento.
     * @post `other.m_pMat == nullptr`, `other.m_rows == 0`, `other.m_cols == 0`.
     * @note Marcado `noexcept`: garantiza compatibilidad con contenedores STL.
     */
    Matrix1(Matrix1 &&other) noexcept;

    /**
     * @brief Asignación de copia eliminada.
     * @see Matrix1(const Matrix1&)
     */
    Matrix1 &operator=(const Matrix1 &other) = delete;

    /**
     * @brief Asignación de movimiento. Libera memoria propia, luego transfiere.
     * @param other Instancia origen.
     * @return Referencia a `*this`.
     * @post `other` queda vacío; `*this` posee los datos originales de `other`.
     * @note Auto-asignación (`this == &other`) se detecta y se omite de forma segura.
     */
    Matrix1 &operator=(Matrix1 &&other) noexcept;

    // ------------------------------------------------------------------
    // Gestión de memoria
    // ------------------------------------------------------------------

    /**
     * @brief Aloca memoria para la matriz según `m_rows` y `m_cols`.
     * @pre  `m_rows > 0` y `m_cols > 0` (normalmente asignados por Read()).
     * @post `m_pMat` apunta a `m_rows` punteros, cada uno a un arreglo de `m_cols` elementos.
     * @warning No inicializa los elementos; los valores son indeterminados hasta ser escritos.
     */
    void Create();

    /**
     * @brief Libera toda la memoria y resetea el objeto a estado vacío.
     * @details Itera sobre las filas eliminando cada `m_pMat[i]`, luego elimina
     *          `m_pMat`. Es seguro llamar a `Destroy()` en un objeto ya vacío.
     * @post `m_pMat == nullptr`, `m_rows == 0`, `m_cols == 0`.
     */
    void Destroy();

    // ------------------------------------------------------------------
    // I/O
    // ------------------------------------------------------------------

    /**
     * @brief Lee dimensiones y elementos desde un stream.
     * @details Formato esperado: `rows cols e00 e01 ... e(R-1)(C-1)`.
     *          Primero llama a `Destroy()` para liberar datos previos, luego a `Create()`.
     * @param is Stream de entrada.
     * @return Referencia al stream para encadenamiento (`is >> a >> b`).
     * @pre El stream debe contener primero dos enteros positivos (rows, cols).
     * @post `m_rows` y `m_cols` reflejan los valores leídos; todos los elementos están inicializados.
     * @see operator>>(istream&, Matrix1<T>&)
     */
    istream &Read(istream &is);

    /**
     * @brief Imprime la matriz al stream en formato legible.
     * @details Salida: `rows cols \\n`, luego cada fila en su propia línea con elementos separados por espacio.
     * @param os Stream de salida.
     * @return Referencia al stream para encadenamiento (`os << a << b`).
     * @see operator<<(ostream&, const Matrix1<T>&)
     */
    ostream &Print(ostream &os) const;

    // ------------------------------------------------------------------
    // Funcional
    // ------------------------------------------------------------------

    /**
     * @brief Aplica una función a cada elemento de la matriz.
     * @details Invoca `func(m_pMat[i][j], args...)` para cada `(i, j)`.
     *          Permite pasar funciones libres, lambdas o functores con cualquier
     *          número de argumentos adicionales.
     *
     * @tparam Func Tipo del callable. Firma esperada: `void func(T&, Args...)`.
     * @tparam Args Tipos de los argumentos adicionales (variadic).
     * @param func  Función o callable a aplicar.
     * @param args  Argumentos adicionales forwarded a cada llamada de `func`.
     *
     * @par Ejemplos
     * @code{.cpp}
     * mat.ApplyFunctionToAll(Square<int>);           // n *= n
     * mat.ApplyFunctionToAll(AddX<int>, 5, 10);      // n += 5 + 10
     * mat.ApplyFunctionToAll(PrintM<int>, cout);     // imprime cada elemento
     * @endcode
     *
     * @see PrintM()
     */
    template <typename Func, typename... Args>
    void ApplyFunctionToAll(Func func, Args&& ...args);

    // ------------------------------------------------------------------
    // Operadores aritméticos
    // ------------------------------------------------------------------

    /**
     * @brief Suma elemento a elemento.
     * @param other Matriz sumando (debe tener las mismas dimensiones).
     * @return Nueva matriz resultado de la suma.
     * @pre `m_rows == other.m_rows && m_cols == other.m_cols`.
     * @note El resultado se crea con `Create()` y se devuelve por movimiento.
     */
    Matrix1 operator+(const Matrix1 &other) const;

    /**
     * @brief Resta elemento a elemento.
     * @param other Sustraendo (debe tener las mismas dimensiones).
     * @return Nueva matriz resultado de la resta.
     * @pre `m_rows == other.m_rows && m_cols == other.m_cols`.
     */
    Matrix1 operator-(const Matrix1 &other) const;

    /**
     * @brief Multiplicación matricial estándar (A × B).
     * @details Calcula `result[i][j] = Σ_k this[i][k] * other[k][j]`.
     *          Complejidad temporal: O(m_rows × other.m_cols × m_cols).
     * @param other Matriz factor derecho.
     * @return Nueva matriz de dimensiones `m_rows × other.m_cols`.
     * @pre `m_cols == other.m_rows` (dimensiones compatibles para multiplicación).
     *
     * @par Ejemplo
     * @code{.cpp}
     * // A(2x3) * B(3x2) = C(2x2)
     * Matrix1<int> A, B;
     * cin >> A >> B;   // A: "2 3 ..." B: "3 2 ..."
     * auto C = A * B;  // C es 2x2
     * @endcode
     */
    Matrix1 operator*(const Matrix1 &other) const;

    /**
     * @brief Multiplicación por escalar (matriz × valor).
     * @param value Escalar por el que se multiplica cada elemento.
     * @return Nueva matriz con cada elemento multiplicado por `value`.
     * @see operator*(T, const Matrix1<T>&)
     */
    Matrix1 operator*(T value) const;

    // ------------------------------------------------------------------
    // Acceso a elementos
    // ------------------------------------------------------------------

    /**
     * @brief Acceso a fila por índice (mutable).
     * @details Devuelve puntero a la fila `i`. El acceso completo `mat[i][j]`
     *          usa el `operator[]` nativo de `T*` para la columna.
     * @param i Índice de fila (0-based).
     * @return Puntero al primer elemento de la fila `i`.
     * @pre `i < m_rows`.
     * @warning Sin comprobación de columna en el segundo `[]`; no exceder `m_cols - 1`.
     */
    T *operator[](size_t i);

    /**
     * @brief Acceso a fila por índice (const).
     * @param i Índice de fila (0-based).
     * @return Puntero const al primer elemento de la fila `i`.
     * @pre `i < m_rows`.
     */
    const T *operator[](size_t i) const;

    // ------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------

    /** @brief Número de filas de la matriz. */
    size_t rows() const { return m_rows; }

    /** @brief Número de columnas de la matriz. */
    size_t cols() const { return m_cols; }
};

// ---------------------------------------------------------------------------
// Implementaciones
// ---------------------------------------------------------------------------

template <typename T>
void Matrix1<T>::Create() {
    assert(m_rows > 0 && m_cols > 0);
    m_pMat = new T *[m_rows];
    for (size_t i = 0; i < m_rows; ++i)
        m_pMat[i] = new T[m_cols];
}

template <typename T>
Matrix1<T>::Matrix1(Matrix1 &&other) noexcept {
    m_pMat = exchange(other.m_pMat, nullptr);
    m_rows = exchange(other.m_rows, 0);
    m_cols = exchange(other.m_cols, 0);
}

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

// ---------------------------------------------------------------------------
// Operadores globales relacionados
// ---------------------------------------------------------------------------

/**
 * @relates Matrix1
 * @brief Operador de extracción de stream. Delega a Matrix1::Read().
 * @tparam T Tipo de elemento de la matriz.
 * @param is  Stream de entrada.
 * @param mat Matriz destino.
 * @return Referencia al stream para encadenamiento.
 */
template <typename T>
istream &operator>>(istream &is, Matrix1<T> &mat) {
    return mat.Read(is);
}

/**
 * @relates Matrix1
 * @brief Operador de inserción de stream. Delega a Matrix1::Print().
 * @tparam T Tipo de elemento de la matriz.
 * @param os  Stream de salida.
 * @param mat Matriz a imprimir (const).
 * @return Referencia al stream para encadenamiento.
 */
template <typename T>
ostream &operator<<(ostream &os, const Matrix1<T> &mat) {
    return mat.Print(os);
}

/**
 * @relates Matrix1
 * @brief Multiplicación escalar × matriz (conmutativa). `5 * mat` delega a `mat * 5`.
 * @tparam T Tipo de elemento.
 * @param value Escalar izquierdo.
 * @param mat   Matriz factor derecho.
 * @return Nueva matriz resultado.
 * @see Matrix1::operator*(T) const
 */
template <typename T>
Matrix1<T> operator*(T value, const Matrix1<T> &mat) {
    return mat * value;
}

/** @} */ // fin del grupo matrix

#endif // __MATRIX1_H__
