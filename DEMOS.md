# Demos: Performance y Regex

Diagramas Mermaid de los dos demos invocados desde `main.cpp`.

---

## DemoPerformance (`performance.cpp`)

Compara multiplicación de matrices `NxN` (N=1000) con 3 estrategias y mide tiempos.

```mermaid
flowchart TD
    A[DemoPerformance] --> B["Reservar A, B, C, Cref<br/>vector T3F plano NxN"]
    B --> C["Llenar A, B con random 1..10<br/>mt19937 seed=42"]
    C --> D["matmul_seq =&gt; Cref<br/>referencia"]
    D --> E[Medir t_seq]

    E --> F["matmul_threads =&gt; C<br/>reparte filas entre hilos"]
    F --> G[Medir t_thr + speedup]
    G --> H{equal C vs Cref?}
    H -->|si| I[OK]
    H -->|no| J[MAL]

    I --> K{compilado con nvcc?}
    J --> K
    K -->|si __CUDACC__| L["matmul_gpu =&gt; C<br/>kernel CUDA 16x16"]
    K -->|no| M[Mensaje: compilar con nvcc]
    L --> N[Medir t_gpu + speedup]
    N --> O[Fin: imprime tabla tiempos]
    M --> O
```

### Detalle multi-hilo (`matmul_threads`)

```mermaid
flowchart LR
    A[N filas] --> B[chunk = N / nthreads]
    B --> C[Por cada hilo t]
    C --> D["r0 = t*chunk<br/>r1 = min(N, r0+chunk)"]
    D --> E["thread matmul_rows<br/>filas r0..r1"]
    E --> F[join todos los hilos]
    F --> G[C completa]
```

### Estrategias comparadas

```mermaid
graph TD
    subgraph SEQ[Secuencial]
        S1[3 loops anidados i,j,k<br/>1 solo hilo]
    end
    subgraph THR[Multi-hilo std thread]
        T1[Filas repartidas<br/>hardware_concurrency hilos]
    end
    subgraph GPU[GPU CUDA opcional]
        G1[1 thread por celda<br/>grid de bloques 16x16]
    end
    SEQ -->|referencia| CMP[Comparar tiempos + validar resultado]
    THR --> CMP
    GPU --> CMP
```

---

## DemoRegex (`regexdemo.cpp`)

4 usos de `std::regex` sobre strings (`TS = std::string`).

```mermaid
flowchart TD
    A[DemoRegex] --> B["1. regex_match<br/>validar email completo"]
    B --> B1["camo2391@gmail.com = valido<br/>no-es-email = invalido"]

    B1 --> C["2. regex_search + grupos<br/>extraer fecha YYYY-MM-DD"]
    C --> C1["m0=fecha, m1=anio,<br/>m2=mes, m3=dia"]

    C1 --> D["3. sregex_iterator<br/>todas coincidencias w+"]
    D --> D1["rojo,verde,azul,amarillo<br/>= rojo verde azul amarillo"]

    D1 --> E["4. regex_replace<br/>sustituir patron"]
    E --> E1["gato gato gato<br/>= perro perro perro"]
```

### Match vs Search vs Iterator vs Replace

```mermaid
graph LR
    M[regex_match] -->|todo el string debe cumplir| MR[bool]
    S[regex_search] -->|primera coincidencia| SR[smatch]
    I[sregex_iterator] -->|todas las coincidencias| IR[loop matches]
    R[regex_replace] -->|sustituye patron| RR[string nuevo]
```
