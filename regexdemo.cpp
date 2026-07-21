#include <iostream>
#include <regex>
#include "types.h"

using namespace std;

// Demo de expresiones regulares con <regex> de la stdlib.
void DemoRegex() {
    cout << "=== Demo Expresiones Regulares (std::regex) ===\n\n";

    // 1. Match completo: validar email
    regex email(R"(^[\w.+-]+@[\w-]+\.[\w.-]+$)");
    for (TS s : {"camo2391@gmail.com", "no-es-email"}) {
        cout << s << " -> "
             << (regex_match(s, email) ? "email valido" : "invalido") << "\n";
    }
    cout << "\n";

    // 2. Search + grupos de captura: extraer fecha
    TS texto = "La fecha es 2026-07-20 y no otra.";
    regex fecha(R"((\d{4})-(\d{2})-(\d{2}))");
    smatch m;
    if (regex_search(texto, m, fecha)) {
        cout << "Fecha encontrada: " << m[0] << "\n";
        cout << "  anio=" << m[1] << " mes=" << m[2] << " dia=" << m[3] << "\n";
    }
    cout << "\n";

    // 3. Iterar todas las coincidencias
    TS csv = "rojo,verde,azul,amarillo";
    regex palabra(R"(\w+)");
    cout << "Palabras: ";
    for (auto it = sregex_iterator(csv.begin(), csv.end(), palabra);
         it != sregex_iterator(); ++it) {
        cout << (*it).str() << " ";
    }
    cout << "\n\n";

    // 4. Reemplazo
    TS frase = "gato gato gato";
    cout << "Reemplazo: " << regex_replace(frase, regex("gato"), "perro") << "\n";
}
