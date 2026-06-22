## @file demo_matrix.py
## @brief Demo de bindings Python de Matrix1<T>.
## Cubre: regex, threads, réplica de DemoPointersMatrix1 (Pointers.cpp).

import re
import threading
from matrix1 import MatrixI, MatrixD

## @brief Regex para formato "rows cols e00 e01 ...". Acepta enteros y decimales.
MATRIX_RE = re.compile(r'^\s*(\d+)\s+(\d+)((?:\s+-?\d+(?:\.\d+)?)+)\s*$')


def parse_matrix_string(s):
    """@brief Valida cadena con formato matricial y verifica conteo de elementos.
    @param s Cadena "rows cols e00 e01 ..."
    @return s sin espacios extremos
    @throws ValueError si formato invalido o conteo incorrecto
    """
    m = MATRIX_RE.match(s.strip())
    if not m:
        raise ValueError(f"Formato invalido: {s!r}")
    rows, cols = int(m.group(1)), int(m.group(2))
    values = m.group(3).split()
    if len(values) != rows * cols:
        raise ValueError(f"Esperados {rows * cols} elementos, encontrados {len(values)}")
    return s.strip()


def demo_regex():
    """@brief Demuestra expresiones regulares para validar strings matriciales."""
    print("=== Demo Regex ===")
    casos = [
        "3 4  1 2 3 4  5 6 7 8  9 10 11 12",  # valida
        "2 2  1.5 2.5  3.5 4.5",               # valida (double)
        "2 3  1 2",                             # invalida: faltan elementos
        "abc",                                  # invalida: sin numeros
    ]
    for s in casos:
        try:
            parse_matrix_string(s)
            print(f"  OK : {s!r}")
        except ValueError as e:
            print(f"  ERR: {e}")


def demo_threads():
    """@brief Calcula m1 = 5*m2 + m3*m4 con dos threads concurrentes.
    Replica la expresion de DemoPointersMatrix1 en Pointers.cpp.
    """
    print("\n=== Demo Threads ===")
    m2 = MatrixI(); m2.from_string(parse_matrix_string("2 2  1 0  0 1"))
    m3 = MatrixI(); m3.from_string(parse_matrix_string("2 3  1 2 3  4 5 6"))
    m4 = MatrixI(); m4.from_string(parse_matrix_string("3 2  7 8  9 10  11 12"))

    results = {}

    def compute_5m2():
        """@brief Thread 1: computa 5 * m2."""
        results['5m2'] = 5 * m2

    def compute_m3m4():
        """@brief Thread 2: computa m3 * m4."""
        results['m3m4'] = m3 * m4

    t1 = threading.Thread(target=compute_5m2)
    t2 = threading.Thread(target=compute_m3m4)
    t1.start(); t2.start()
    t1.join();  t2.join()

    m1 = results['5m2'] + results['m3m4']
    print("m1 = 5*m2 + m3*m4 (via threads):")
    print(m1)


def demo_pointers_matrix1():
    """@brief Replica Python de DemoPointersMatrix1() de Pointers.cpp.
    Cubre: ApplyFunctionToAll equivalente, operadores aritméticos y acceso [][].
    """
    print("\n=== Demo Pointers Matrix1 (Python) ===")
    mat = MatrixI()
    mat.from_string(parse_matrix_string("3 4  1 2 3 4  5 6 7 8  9 10 11 12"))

    print("Matriz original:")
    print(mat)

    print("Aplicando cuadrado:")
    for i in range(mat.rows):
        for j in range(mat.cols):
            mat[i][j] = mat[i][j] * mat[i][j]
    print(mat)

    print("Sumandole valores extras (5 + 10):")
    for i in range(mat.rows):
        for j in range(mat.cols):
            mat[i][j] = mat[i][j] + 15
    print(mat)

    print("\nDemo operadores: m1 = 5*m2 + m3*m4")
    m2 = MatrixI(); m2.from_string("2 2  1 0  0 1")
    m3 = MatrixI(); m3.from_string("2 3  1 2 3  4 5 6")
    m4 = MatrixI(); m4.from_string("3 2  7 8  9 10  11 12")

    print("m2:"); print(m2)
    print("m3:"); print(m3)
    print("m4:"); print(m4)

    m1 = 5 * m2 + m3 * m4
    print("m1 = 5*m2 + m3*m4:"); print(m1)

    print("Demo operador [][]: lectura y escritura")
    print(f"m1[0][0]={m1[0][0]}  m1[1][1]={m1[1][1]}")
    m1[0][0] = 999
    print("Tras m1[0][0]=999:"); print(m1)

    print("Demo [][] solo lectura:")
    print(f"m1[0][0]={m1[0][0]}  m1[1][1]={m1[1][1]}")


# ---- Demos originales ----

m1 = MatrixI()
m1.from_string("2 3  1 2 3  4 5 6")

m2 = MatrixI()
m2.from_string("2 3  7 8 9  10 11 12")

m3 = MatrixI()
m3.from_string("3 2  1 0  0 1  2 3")

print("m1:")
print(m1)

print("m2:")
print(m2)

m_sum = m1 + m2
print("m1 + m2:")
print(m_sum)

m_diff = m2 - m1
print("m2 - m1:")
print(m_diff)

m_prod = m1 * m3
print("m1 * m3  (2x3 * 3x2 = 2x2):")
print(m_prod)

m_scaled = m1 * 3
print("m1 * 3:")
print(m_scaled)

m_rscaled = 5 * m1
print("5 * m1:")
print(m_rscaled)

print("m1[0][0] =", m1[0][0])
print("m1[1][2] =", m1[1][2])

m1[0][0] = 999
print("Despues de m1[0][0] = 999:")
print(m1)

md = MatrixD()
md.from_string("2 2  1.5 2.5  3.5 4.5")
print("md (double):")
print(md)

md_prod = md * 2.0
print("md * 2.0:")
print(md_prod)

# ---- Nuevos demos ----
demo_regex()
demo_threads()
demo_pointers_matrix1()
