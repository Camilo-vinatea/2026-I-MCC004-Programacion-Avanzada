# PROG-AVZ-MCC-004 — Programación Avanzada

Repositorio del curso de Programación Avanzada, Maestría en Ciencias de la Computación (UNI).  
Cubre gestión de memoria dinámica, templates, polimorfismo y operadores en C++11/23.

## Compilación

```bash
g++ -std=c++2b main.cpp util.cpp -o main
./main
```

## Estructura del proyecto

```
.
├── main.cpp              # Punto de entrada — activa demos por línea
├── types.h               # Aliases de tipos (TI, TP, RealType, ...)
├── util.h / util.cpp     # Utilidades generales
│
├── array1.h / .cpp       # Nivel 1: funciones C-style para arreglos TP*
├── array2.h              # Nivel 2: plantilla con stream I/O
├── array3.h              # Nivel 3: clase con ApplyFunctionToAll
├── array4.h              # Nivel 4: clase genérica con operator<< / >>
│
├── matrix1.h             # Matriz 2D dinámica (heap), plantilla completa
├── matrix1_py.cpp        # Bindings pybind11 de Matrix1<T> para Python
├── setup.py              # Compilación del módulo Python (setuptools)
├── demo_matrix.py        # Demo Python: regex, threads, réplica DemoPointersMatrix1
│
├── complex.h / .cpp      # Clase Complex con semántica de valor
├── BitSigno.h / .cpp     # Manipulación de bit de signo
│
├── shapes/               # Jerarquía polimórfica
│   ├── shape.h / .cpp    #   Shape (base abstracta — GetArea() virtual puro)
│   ├── circle.h / .cpp
│   ├── rectangle.h / .cpp
│   ├── square.h / .cpp
│   └── triangle.h / .cpp
│
├── polimorfismo.h / .cpp # Demo de polimorfismo con shapes
├── functions.h / .cpp    # Funciones de utilidad y demos
└── Pointers.h / .cpp     # Demos de punteros, referencias y matrices
```

## Módulos principales

### `Array1` → `Array4` — Evolución del arreglo dinámico

Progresión didáctica desde funciones C-style hasta clase template con streams:

| Nivel | Archivo  | Característica clave                              |
|-------|----------|---------------------------------------------------|
| 1     | array1   | Funciones libres, tipo fijo `TP*`                 |
| 2     | array2   | Template + stream I/O paramétrico                 |
| 3     | array3   | Clase + `ApplyFunctionToAll` con variadic args    |
| 4     | array4   | `operator>>` / `operator<<` + move semantics      |

### `Matrix1<T>` — Matriz 2D dinámica

Almacenamiento interno por filas en heap (`T**`). Implementa la regla de 5:

```
m_pMat → [ fila0* | fila1* | ... ]
               ↓        ↓
           [e00 e01] [e10 e11] ...
```

**Operaciones disponibles:**

```cpp
Matrix1<int> m1, m2, m3, m4;

m1 = 5 * m2 + m3 * m4;   // escalar×matriz, producto matricial, suma
m1 = m3 - m2;             // resta elemento a elemento
m2.ApplyFunctionToAll(Square<int>);        // transforma cada elemento
m2.ApplyFunctionToAll(AddX<int>, 5, 10);  // con argumentos extras

cin  >> m1;   // lee "rows cols e00 e01 ..."
cout << m1;   // imprime "rows cols \n fila0 \n fila1 \n ..."
```

**Regla de 5 completa:**

```cpp
Matrix1(const Matrix1 &other);            // copy constructor — deep copy
Matrix1(Matrix1 &&other) noexcept;        // move constructor
Matrix1 &operator=(const Matrix1 &other); // copy assignment
Matrix1 &operator=(Matrix1 &&other) noexcept; // move assignment
~Matrix1();                               // destructor — libera filas + array
```

### `Shape` — Polimorfismo

```
Shape  (GetArea() = 0, ToString(), operator<<)
  ├── Circle
  ├── Rectangle
  │     └── Square
  └── Triangle
```

## Demos activadas en `main.cpp`

Descomentar la línea correspondiente para ejecutar cada demo:

```cpp
// DemoFunctions();
// DemoComplex();
// DemoPolimorfismo();
// BitSigno();
// DemoPointers1();          // semántica de valor/referencia/puntero (f1-f7)
// DemoPointersVector1();    // arreglo C-style
// DemoPointersVector2();    // template + stream
// DemoPointersVector3();    // clase Array3
// DemoPointersVector4();    // clase Array4
// DemoPointersVector5();    // Array4 con operator<< / >>
DemoPointersMatrix1();       // Matrix1 + operadores aritméticos
```

## Bindings Python — `matrix1`

`Matrix1<int>` y `Matrix1<double>` expuestos como `MatrixI` / `MatrixD` vía pybind11.

### Compilar el módulo

```bash
pip install pybind11 setuptools
python setup.py build_ext --inplace
```

O con el Makefile:

```bash
make python_module
```

### Usar en Python

```python
from matrix1 import MatrixI, MatrixD

m = MatrixI()
m.from_string("2 3  1 2 3  4 5 6")
print(m.rows, m.cols)   # 2 3
print(m)

m2 = MatrixI(); m2.from_string("2 3  7 8 9  10 11 12")
print(m + m2)           # suma elemento a elemento
print(5 * m)            # escalar × matriz
print(m[0][1])          # lectura m[i][j]
m[0][1] = 99            # escritura m[i][j]
```

### `demo_matrix.py` — features demostradas

| Función | Descripción |
|---|---|
| `parse_matrix_string(s)` | Valida formato `"rows cols e00 ..."` con `re`; lanza `ValueError` si inválido |
| `demo_regex()` | Prueba 4 casos (2 OK, 2 error) con `MATRIX_RE` |
| `demo_threads()` | Calcula `5*m2` y `m3*m4` en threads separados, suma resultados |
| `demo_pointers_matrix1()` | Réplica de `DemoPointersMatrix1()` (Pointers.cpp): cuadrado, AddX, operadores, `[][]` |

Toda función tiene docstring con tags Doxygen (`@brief`, `@param`, `@return`, `@throws`).

### Patrón de punteros a método (dispatch)

`matrix1_py.cpp` usa punteros a método explícitamente para satisfacer requerimiento académico:

```cpp
// dispatch via .*
Matrix1<T> dispatch(Matrix1<T>& lhs, const Matrix1<T>& rhs, BinOp<T> method) {
    return (lhs.*method)(rhs);   // equivalente a lhs + rhs
}

// dispatch via ->*
Matrix1<T> dispatch_ptr(Matrix1<T>* pLhs, const Matrix1<T>& rhs, BinOp<T> method) {
    return (pLhs->*method)(rhs); // equivalente a (*pLhs) + rhs
}
```

## Ramas del repositorio

| Rama          | Contenido                        |
|---------------|----------------------------------|
| `01-types`    | Aliases de tipos                 |
| `09-BitdeSigno` | Clase BitSigno                 |
| `10-Pointers1`  | Demos de punteros y referencias |
| `12-Matrix`     | Matrix1 con Rule of 5 y operadores |
