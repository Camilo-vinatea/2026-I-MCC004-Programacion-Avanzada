// performance.cpp
// Compara multiplicacion de matrices NxN (elementos aleatorios 1..10):
//   1) Secuencial
//   2) Multi-hilo (std::thread)
//   3) GPU (CUDA)  -- solo se compila con nvcc
//
// Se invoca desde main.cpp via DemoPerformance().
// Compilar con GPU: nvcc -O2 -std=c++17 -x cu performance.cpp -o performance
//
// ponytail: matrices como vector<T3F> plano (row-major). Sin clase Matrix:
// el demo solo mide tiempo, no necesita la abstraccion.

#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <cstdio>
#include <cmath>
#include "types.h"

constexpr TI N = 1000;

using Clock = std::chrono::high_resolution_clock;
static double ms_since(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// ---- Secuencial ------------------------------------------------------------
static void matmul_seq(const T3F* A, const T3F* B, T3F* C) {
    for (TI i = 0; i < N; ++i)
        for (TI j = 0; j < N; ++j) {
            T3F s = 0.0f;
            for (TI k = 0; k < N; ++k) s += A[i * N + k] * B[k * N + j];
            C[i * N + j] = s;
        }
}

// ---- Multi-hilo: reparte filas entre hilos ---------------------------------
static void matmul_rows(const T3F* A, const T3F* B, T3F* C, TI r0, TI r1) {
    for (TI i = r0; i < r1; ++i)
        for (TI j = 0; j < N; ++j) {
            T3F s = 0.0f;
            for (TI k = 0; k < N; ++k) s += A[i * N + k] * B[k * N + j];
            C[i * N + j] = s;
        }
}

static void matmul_threads(const T3F* A, const T3F* B, T3F* C, TI nthreads) {
    std::vector<std::thread> pool;
    TI chunk = (N + nthreads - 1) / nthreads;
    for (TI t = 0; t < nthreads; ++t) {
        TI r0 = t * chunk, r1 = std::min(N, r0 + chunk);
        if (r0 >= r1) break;
        pool.emplace_back(matmul_rows, A, B, C, r0, r1);
    }
    for (auto& th : pool) th.join();
}

// ---- GPU (CUDA) ------------------------------------------------------------
#ifdef __CUDACC__
__global__ void matmul_kernel(const T3F* A, const T3F* B, T3F* C) {
    TI row = blockIdx.y * blockDim.y + threadIdx.y;
    TI col = blockIdx.x * blockDim.x + threadIdx.x;
    if (row < N && col < N) {
        T3F s = 0.0f;
        for (TI k = 0; k < N; ++k) s += A[row * N + k] * B[k * N + col];
        C[row * N + col] = s;
    }
}

static double matmul_gpu(const T3F* A, const T3F* B, T3F* C) {
    T3F *dA, *dB, *dC;
    size_t bytes = size_t(N) * N * sizeof(T3F);
    cudaMalloc(&dA, bytes); cudaMalloc(&dB, bytes); cudaMalloc(&dC, bytes);
    cudaMemcpy(dA, A, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(dB, B, bytes, cudaMemcpyHostToDevice);

    dim3 block(16, 16);
    dim3 grid((N + 15) / 16, (N + 15) / 16);

    auto t0 = Clock::now();
    matmul_kernel<<<grid, block>>>(dA, dB, dC);
    cudaDeviceSynchronize();           // mide solo el kernel (sin transferencias)
    double t = ms_since(t0);

    cudaMemcpy(C, dC, bytes, cudaMemcpyDeviceToHost);
    cudaFree(dA); cudaFree(dB); cudaFree(dC);
    return t;
}
#endif

// ---- Utilidades ------------------------------------------------------------
static bool equal(const T3F* X, const T3F* Y) {
    for (TI i = 0; i < N * N; ++i)
        if (std::fabs(X[i] - Y[i]) > 1e-2f) return false;
    return true;
}

void DemoPerformance() {
    std::vector<T3F> A(N * N), B(N * N), C(N * N), Cref(N * N);

    std::mt19937 rng(42);
    std::uniform_int_distribution<TI> dist(1, 10);
    for (TI i = 0; i < N * N; ++i) { A[i] = T3F(dist(rng)); B[i] = T3F(dist(rng)); }

    std::printf("Multiplicacion de matrices %dx%d (elementos 1..10)\n\n", N, N);

    // Secuencial (referencia)
    auto t0 = Clock::now();
    matmul_seq(A.data(), B.data(), Cref.data());
    double t_seq = ms_since(t0);
    std::printf("Secuencial      : %8.3f ms\n", t_seq);

    // Multi-hilo
    TI nthreads = std::max<TI>(1, std::thread::hardware_concurrency());
    t0 = Clock::now();
    matmul_threads(A.data(), B.data(), C.data(), nthreads);
    double t_thr = ms_since(t0);
    std::printf("Multi-hilo (%2dh): %8.3f ms  (speedup %.2fx, %s)\n",
                nthreads, t_thr, t_seq / t_thr, equal(C.data(), Cref.data()) ? "OK" : "MAL");

#ifdef __CUDACC__
    double t_gpu = matmul_gpu(A.data(), B.data(), C.data());
    std::printf("GPU (CUDA)      : %8.3f ms  (speedup %.2fx, %s)\n",
                t_gpu, t_seq / t_gpu, equal(C.data(), Cref.data()) ? "OK" : "MAL");
#else
    std::printf("GPU (CUDA)      :   (compilar con nvcc para medir)\n");
#endif
}
