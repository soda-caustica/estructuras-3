Procesamiento de Tweets y Análisis Comparativo de Tablas Hash

Implementación en C++17 de tres tablas hash (hashing abierto, hashing cerrado con tres estrategias de sondeo, y un wrapper de std::unordered_map de la STL) para el conteo de tweets por usuario, comparando dos tipos de clave (user_id y user_screen_name).

Entregable 3 — Estructuras de Datos 2026.
Integrantes: Nicolás Ricciardi, Enzo Levancini, Máximo Beltrán

--------------------------------------------------------------------------------

Requisitos

- MinGW-w64 / GCC 14.x o superior (soporte C++17)
- Bash (Git Bash en Windows, o Linux/macOS)
- Python 3.x (para análisis y gráficos)
- Librerías Python: pandas, matplotlib

Para instalar las dependencias de Python necesarias para la generación de gráficos:
pip install pandas matplotlib

--------------------------------------------------------------------------------

Compilar y Ejecutar

El proyecto no usa CMake: se compila directamente con g++ y se corre mediante un script Bash. No hay una versión "manual" alternativa de correr todos los experimentos: el script es el único orquestador.

1. Descargar el dataset

El CSV (auspol2019.csv, ~70 MB) no está incluido en el repositorio por su tamaño. Debe colocarse en src/dataset/auspol2019.csv (ver enlace del enunciado).

2. Ejecución Automatizada (Linux / macOS / Git Bash en Windows)

run_experimentos.sh compila src/main.cpp en modo Release (-O2, crítico para mediciones de tiempo representativas) y genera un ejecutable ./experimento parametrizado por línea de comandos:

    ./experimento <estructura> <clave> tiempo <numero_experimento>
    ./experimento <estructura> <clave> memoria

Cada invocación mide UNA sola combinación (una repetición de tiempo, o la pasada única de memoria) y devuelve sus filas por stdout; el script es quien repite 20 veces por combinación y arma los CSV finales:

# Otorgar permisos de ejecución (si estás en Linux/macOS)
chmod +x run_experimentos.sh

# Ejecutar el entorno completo (compila + corre las 200 mediciones de tiempo + 10 de memoria)
./run_experimentos.sh

3. Generación de Gráficos de Rendimiento

Una vez que el script termine y guarde las mediciones en formato CSV dentro de la carpeta results/, ejecuta el script de análisis en Python para calcular promedios/desviación estándar y generar las curvas de escalabilidad y las comparativas de memoria:

python analisis/analizar_resultados.py

*Los gráficos PNG resultantes se almacenarán automáticamente en la carpeta results/plots/, y el resumen estadístico en results/results_tiempos_resumen.csv.*

## 📂 Estructura del Proyecto

```text
📦 ESTRUCTURAS-3
 ┣ 📂 analisis
 ┃ ┗ 📜 analizar_resultados.py       (Promedio/desv. estándar por checkpoint y gráficos comparativos)
 ┣ 📂 Informe
 ┃ ┗ 📜 InformeTarea3_TablasHash.pdf (Documento final en PDF con el estudio experimental)
 ┣ 📂 results
 ┃ ┣ 📂 plots                        (Gráficos generados: memoria y curvas de tiempo)
 ┃ ┃ ┣ 📜 memoria_screen_name.png
 ┃ ┃ ┣ 📜 memoria_user_id.png
 ┃ ┃ ┣ 📜 tiempo_screen_name.png
 ┃ ┃ ┗ 📜 tiempo_user_id.png
 ┃ ┣ 📜 results_memoria.csv          (Memoria por checkpoint, una sola medición por combinación)
 ┃ ┣ 📜 results_tiempos.csv          (Tiempos crudos: 20 repeticiones x 10 combinaciones x 18 checkpoints)
 ┃ ┗ 📜 results_tiempos_resumen.csv  (Promedios y desviaciones estándar calculados)
 ┣ 📂 src
 ┃ ┣ 📂 dataset                      (auspol2019.csv - no incluido en el repo, ver enunciado)
 ┃ ┣ 📂 experimentos
 ┃ ┃ ┣ 📜 dataset_loader.h           (Parser del CSV: carga el archivo completo a memoria y extrae user_id/user_screen_name)
 ┃ ┃ ┗ 📜 experimento.h              (Mide UNA pasada de tiempo o de memoria; la repetición la maneja run_experimentos.sh)
 ┃ ┣ 📂 tablas
 ┃ ┃ ┣ 📜 tabla_hashing_abierto.hpp  (Encadenamiento con listas enlazadas; factor de carga máximo 1.0)
 ┃ ┃ ┣ 📜 tabla_hashing_cerrado.hpp  (Direccionamiento abierto: Linear, Quadratic y Double Probing; factor de carga máximo 0.5)
 ┃ ┃ ┗ 📜 tabla_unordered_map.hpp    (Wrapper de std::unordered_map: agrega memoryBytes() para comparar memoria contra las otras 2 estructuras)
 ┃ ┣ 📂 utils
 ┃ ┃ ┣ 📜 csv_reader.h               (Parser CSV genérico según RFC 4180: comillas, comas y saltos de línea dentro de campos)
 ┃ ┃ ┣ 📜 hash_functions.h           (SplitMix64 + component sum para user_id; acumulación polinomial + FNV-1a para user_screen_name)
 ┃ ┃ ┣ 📜 linked_list.h              (Lista enlazada usada como bucket del hashing abierto)
 ┃ ┃ ┣ 📜 memory_utils.h             (Estimación de bytes extra en el heap que ocupa una clave, ej. buffer de un std::string)
 ┃ ┃ ┣ 📜 prime_utils.h              (Búsqueda de primos para el crecimiento de la capacidad en cada rehash)
 ┃ ┃ ┗ 📜 timer.h                    (Cronómetro basado en std::chrono::steady_clock)
 ┃ ┗ 📜 main.cpp                     (Ejecutable parametrizado por argv: mide una sola combinación estructura+clave por invocación)
 ┣ 📜 .gitignore
 ┗ 📜 run_experimentos.sh            (Compila y orquesta las 210 invocaciones del ejecutable, arma los CSV finales)
```

Metodología y Diseño Experimental

El programa analiza la escalabilidad de las estructuras sobre el dataset de tweets de las elecciones australianas de 2019 (183.370 tweets, 45.862 usuarios únicos), midiendo en checkpoints de 10.000 en 10.000 hasta 180.000.

Esquema de Conteo
Para cada fila del dataset, se evalúa el par de claves bajo la regla solicitada:
if (k en H) {
    H[k] = H[k] + 1;
} else {
    H[k] = 1;
}

Parámetros e Implementación de las Estructuras

1. Tabla Hashing Abierto: manejo de colisiones por encadenamiento (lista enlazada por bucket). Factor de carga máximo 1,0; al superarlo se duplica la capacidad al siguiente número primo y se redistribuyen las claves (rehash).

2. Tabla Hashing Cerrado: direccionamiento abierto. Factor de carga máximo 0,5 (garantiza que el sondeo cuadrático siempre encuentre una celda libre). Resuelve colisiones mediante tres modalidades inyectadas en el constructor:
   - Linear Probing: index = (h1(k) + j) % N
   - Quadratic Probing: index = (h1(k) + j^2) % N
   - Double Hashing: index = (h1(k) + j * paso) % N, con paso = q - (h2(k) % q) y q el mayor primo menor que N

3. Funciones Hash Utilizadas:
   - Para user_id (uint64_t): hash principal SplitMix64 (mezclador multiplicativo de bits). Hash secundario (solo para double hashing) component sum, que suma los 8 bytes del entero.
   - Para user_screen_name (std::string): hash principal por acumulación polinomial (regla de Horner, base Z = 31). Hash secundario (solo para double hashing) FNV-1a de 64 bits.

Rigor del Muestreo
- Cada combinación de estructura y clave se mide en checkpoints de 10.000 en 10.000 tweets (10k, 20k, ..., 180k), repitiendo cada medición 20 veces.
- Se reportan la media aritmética y la desviación estándar de los tiempos de ejecución para cada checkpoint (results_tiempos_resumen.csv).
- La memoria se mide en una pasada aparte de las cronometradas (una sola vez, no 20): su valor es determinista dado que el orden de inserción no cambia entre repeticiones.
- Formato de salida: CSV separado por ';' (numero_experimento;dataset;estructura_de_datos;clave;cantidad_consultas;tiempo_ejecucion), según lo solicitado en el enunciado.
