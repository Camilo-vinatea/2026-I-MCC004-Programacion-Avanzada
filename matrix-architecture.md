# Matrices Architecture - PR #12

## Evolution: Array → Matrix

```mermaid
flowchart TB
    subgraph Arrays["Arrays (1D - heap)"]
        A1["Array1<br/>TP* + functions<br/>No class encapsulation"]
        A2["Array2<br/>TI* + functions + stream params"]
        A3["Array3<br/>Template class<br/>ApplyFunctionToAll"]
        A4["Array4<br/>Template class<br/>Stream operators >> <<"]
    end

    subgraph Matrix["Matrix1 (2D - heap)"]
        M1["Matrix1<T><br/>T** m_pMat → [row0*, row1*, ...]<br/>Create() → new T*[rows], then new T[cols] per row"]
        M1F["Member Functions"]
        M1F --- M1Create["Create() - allocate rows then cols"]
        M1F --- M1Read["Read(stream) - read rows cols, Create(), read elements"]
        M1F --- M1Print["Print(stream) - format: rows cols \\n elements"]
        M1F --- M1Destroy["Destroy() - delete[] each row, delete[] rowptrs"]
        M1F --- M1Apply["ApplyFunctionToAll(func, args...) - variadic template"]
    end

    M1Ops["Operators"]
    M1Ops --- M1Add["operator+ (element-wise)"]
    M1Ops --- M1Sub["operator- (element-wise)"]
    M1Ops --- M1MulMat["operator* (matrix multiplication)"]
    M1Ops --- M1MulScal["operator* (scalar)"]
    M1Ops --- M1In["operator>> (calls Read)"]
    M1Ops --- M1Out["operator<< (calls Print)"]
    M1Ops --- M1ScalMul["operator* (scalar, lhs) - 5 * mat"]

    M1 --> M1Ops

    Arrays -->|"Template class<br/>evolution"| M1

    style Arrays fill:#1a1a2e,color:#fff
    style Matrix fill:#16213e,color:#fff
    style M1Ops fill:#0f3460,color:#fff
```

## Internal Storage Structure

```mermaid
graph LR
    subgraph Memory["Memory Layout"]
        POINTERS["m_pMat (T**)<br/>heap allocated"]
        ROW0["row 0: [e00 e01 e02 ... e0(C-1)]"]
        ROW1["row 1: [e10 e11 e12 ... e1(C-1)]"]
        ROW2["..."]
        ROWn["row (R-1): [(R-1)0 (R-1)1 ...]"]

        POINTERS --> ROW0
        POINTERS --> ROW1
        POINTERS --> ROW2
        POINTERS --> ROWn
    end

    style Memory fill:#0f3460,color:#fff
```

## Class Definition

```mermaid
classDiagram
    class Matrix1~T~ {
        +T** m_pMat
        +size_t m_rows
        +size_t m_cols
        +Matrix1()
        +~Matrix1()
        +Matrix1(copy)
        +Matrix1(move)
        +operator=(copy)
        +operator=(move)
        +Create()
        +Read(istream)
        +ApplyFunctionToAll(func, args...)
        +Print(ostream)
        +Destroy()
        +operator+(Matrix1)
        +operator-(Matrix1)
        +operator*(Matrix1)
        +operator*(T)
    }

    Matrix1 --|> "copyable"
    Matrix1 --|> "movable"
```

## Data Flow

```mermaid
flowchart LR
    SS["istringstream<br/>'3 4 1 2 3 4 5 6 7 8 9 10 11 12'"]
    SS -->|">>"| READ["Read()"]
    READ -->|rows=3 cols=4| CREATE["Create()"]
    CREATE -->|new T*[3]| PTR["m_pMat"]
    CREATE -->|new T[4] x3| ROWS["row allocations"]
    READ -->|read elements| ROWS

    PTR -->|m_pMat[0]| R0["[1 2 3 4]"]
    PTR -->|m_pMat[1]| R1["[5 6 7 8]"]
    PTR -->|m_pMat[2]| R2["[9 10 11 12]"]

    style SS fill:#1a1a2e,color:#fff
    style READ fill:#16213e,color:#fff
    style CREATE fill:#16213e,color:#fff
    style PTR fill:#0f3460,color:#fff
    style ROWS fill:#0f3460,color:#fff
```

## Matrix Multiplication (A * B)

```mermaid
flowchart LR
    subgraph A["Matrix A (MxK)"]
        A0["m_rows=M, m_cols=K"]
    end
    subgraph B["Matrix B (KxN)"]
        B0["m_rows=K, m_cols=N"]
    end
    subgraph R["Result (MxN)"]
        R0["m_rows=M, m_cols=N"]
        R0 -->|R[i][j] = sum| SUM["Σ A[i][k] * B[k][j]"]
    end

    A -->|cols=K| SUM
    B -->|rows=K| SUM
    SUM --> R

    style A fill:#1a1a2e,color:#fff
    style B fill:#1a1a2e,color:#fff
    style R fill:#16213e,color:#fff
```

## Type Aliases

```mermaid
graph TD
    T["types.h"]
    T --> TI["using TI = int"]
    T --> TP["using TP = int"]
    T --> T1["using T1 = int32_t"]
    T --> T5["using T5 = double"]
    T --> T8["using T8 = int"]

    TI -->|used in| M1Demo["DemoPointersMatrix1()<br/>Matrix1<TI> mat;"]

    style T fill:#1a1a2e,color:#fff
    style TI fill:#0f3460,color:#fff
```