/**
 * @file demo-cpp.cpp
 * @brief Funciones extraídas del repositorio demo-cpp que ilustran características
 *        modernas de C++17: conversión segura de tipos y suma paralela recursiva.
 *
 * Estas funciones provienen del proyecto demo-cpp (DemoCPP/DemoCPP/demo.cpp)
 * y se documentan aquí como referencia de buenas prácticas en C++ moderno.
 *
 * @author Camilo
 * @date 2026
 */

#include <optional>
#include <string_view>
#include <charconv>    // std::from_chars
#include <future>      // std::async
#include <numeric>     // std::accumulate

// ─────────────────────────────────────────────────────────────────────────────
// Función 1: convert<T>
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Convierte una cadena de texto a un valor numérico de forma segura.
 *
 * Utiliza `std::from_chars` (C++17) para intentar la conversión sin lanzar
 * excepciones. Si la cadena no representa un número válido del tipo @p T,
 * retorna `std::nullopt` en lugar de un valor indefinido.
 *
 * Características C++17 que emplea:
 * - `std::optional<T>` como tipo de retorno que modela "valor o ausencia".
 * - `std::string_view` para evitar copias del string de entrada.
 * - `std::from_chars` para conversión de bajo nivel sin locale ni excepciones.
 * - *Structured bindings* (`auto [ptr, ec]`) para desempacar el resultado de
 *   `from_chars` en dos variables con nombres descriptivos.
 *
 * @tparam T  Tipo numérico destino (e.g. `int`, `long`, `float`).
 *            Debe ser compatible con `std::from_chars`.
 *
 * @param[in]  sv   Vista de la cadena a convertir (no se copia).
 * @param[out] val  Variable donde se almacena el resultado si la conversión
 *                  es exitosa. Su valor queda indefinido si se retorna `nullopt`.
 *
 * @return `std::optional<T>` con el valor convertido si la cadena es válida,
 *         o `std::nullopt` si ocurre cualquier error de formato.
 *
 * @note La función solo acepta representaciones decimales enteras cuando @p T
 *       es un tipo entero. Prefijos como `"0x"` (hex) o espacios iniciales
 *       causan retorno de `nullopt`.
 *
 * @par Ejemplo de uso:
 * @code{.cpp}
 * int val;
 * auto result = convert<int>("42", val);
 * if (result)
 *     std::cout << "Convertido: " << *result << "\n";  // imprime 42
 * else
 *     std::cout << "Conversión fallida\n";
 *
 * // Casos que retornan nullopt:
 * convert<int>("  077", val);  // espacios iniciales → nullopt
 * convert<int>("hello", val);  // no numérico        → nullopt
 * convert<int>("0x33",  val);  // prefijo hex        → nullopt
 * @endcode
 *
 * @see DemoConvert() para ejemplos de todas las conversiones probadas.
 * @see std::from_chars en https://en.cppreference.com/w/cpp/utility/from_chars
 */
template <typename T>
std::optional<T> convert(std::string_view sv, T& val)
{
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
    if (ec != std::errc{})
        return std::nullopt;
    return val;
}

// ─────────────────────────────────────────────────────────────────────────────
// Función 2: parallel_sum
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Calcula la suma de un rango numérico dividiendo el trabajo en hilos.
 *
 * Implementa el patrón *divide y vencerás* usando `std::async` para lanzar
 * tareas asíncronas. El rango se parte recursivamente por la mitad; cuando el
 * tamaño es menor al umbral (1 000 elementos) se resuelve directamente con
 * `std::accumulate`, evitando la sobrecarga de crear hilos para rangos pequeños.
 *
 * Esquema de ejecución para N = 4 000:
 * @verbatim
 *   parallel_sum([0..3999])
 *   ├─ async → parallel_sum([2000..3999])   ← hilo nuevo
 *   └─ recursivo → parallel_sum([0..1999])
 *                  ├─ async → parallel_sum([1000..1999])
 *                  └─ recursivo → accumulate([0..999])   ← umbral alcanzado
 * @endverbatim
 *
 * @tparam RandomIt  Iterador de acceso aleatorio (e.g. `std::vector<int>::iterator`).
 *                   Debe apuntar a un tipo sumable con `operator+`.
 *
 * @param[in] beg  Iterador al primer elemento del rango (inclusive).
 * @param[in] end  Iterador al elemento siguiente al último (exclusive).
 *
 * @return Suma entera (`int`) de todos los elementos en `[beg, end)`.
 *
 * @pre El rango `[beg, end)` debe ser válido (beg <= end).
 * @pre Los elementos deben ser convertibles a `int` sin pérdida de precisión.
 *
 * @warning El número de hilos creados crece como O(N/1000). Para rangos muy
 *          grandes esto puede saturar el planificador del sistema operativo.
 *          Considerar `std::execution::par` de C++17 para uso en producción.
 *
 * @par Complejidad:
 * - **Tiempo:** O(N) trabajo total; O(N/1000 · log₂(N/1000)) en paralelo ideal.
 * - **Espacio:** O(log₂(N/1000)) frames recursivos en el hilo principal.
 *
 * @par Ejemplo de uso:
 * @code{.cpp}
 * std::vector<int> v(1'000'000, 1);
 * int total = parallel_sum(v.begin(), v.end());
 * std::cout << "Suma: " << total << "\n";  // imprime 1000000
 * @endcode
 *
 * @see DemoMutex() donde se invoca esta función junto a otros ejemplos de
 *      concurrencia con `std::async` y `std::mutex`.
 * @see std::async en https://en.cppreference.com/w/cpp/thread/async
 */
template <typename RandomIt>
int parallel_sum(RandomIt beg, RandomIt end)
{
    auto len = end - beg;
    if (len < 1000)
        return std::accumulate(beg, end, 0);

    RandomIt mid = beg + len / 2;
    auto handle = std::async(std::launch::async, parallel_sum<RandomIt>, mid, end);
    int sum = parallel_sum(beg, mid);
    return sum + handle.get();
}
