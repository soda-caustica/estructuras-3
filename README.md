Procesamiento de Tweets y Análisis Comparativo de Tablas Hash

Implementación en C++17 de diferentes estructuras de tablas hash (Abierta, Cerrada con tres tipos de exploración y STL) para el conteo incremental de tweets por usuario utilizando dos variantes de claves (user_id y user_screen_name).

Entregable 3 — Estructuras de Datos 2026.
Integrantes: Nicolás Ricciardi, Enzo Levancini, Máximo Beltrán

--------------------------------------------------------------------------------

Requisitos

- CMake 3.16 o superior
- MinGW-w64 / GCC 14.x o superior
- Python 3.x (para análisis y gráficos)
- Librerías Python: pandas, matplotlib, seaborn

Para instalar las dependencias de Python necesarias para la generación de gráficos:
pip install pandas matplotlib seaborn

--------------------------------------------------------------------------------

Compilar y Ejecutar

Nota importante: El proyecto está automatizado para entornos Unix o Git Bash mediante un script, pero puedes compilarlo manualmente usando CMake o G++ (C++17).

1. Ejecución Automatizada (Linux / macOS / Git Bash en Windows)

El repositorio incluye un script en Bash (run_experimentos.sh) que compila el código fuente en modo Release (con la optimización -O2, la cual es crítica para obtener mediciones de tiempo precisas y representativas) y ejecuta todo el entorno experimental de forma secuencial.

# Otorgar permisos de ejecución (si estás en Linux/macOS)
chmod +x run_experimentos.sh

# Ejecutar el entorno completo
./run_experimentos.sh


2. Generación de Gráficos de Rendimiento

Una vez que el ejecutable de C++ termine y guarde las mediciones en formato CSV dentro de la carpeta results/, ejecuta el script de automatización en Python para generar las curvas de escalabilidad y las comparativas de memoria:

python analisis/analizar_resultados.py

*Los gráficos PNG resultantes se almacenarán automáticamente en la carpeta results/plots/.*

--------------------------------------------------------------------------------

Estructura del Proyecto

ESTRUCTURAS-3
 |-- analisis
 |   |-- analizar_resultados.py       (Script de Python para procesar CSVs y generar gráficos)
 |-- Informe
 |   |-- InformeTarea3_TablasHash.pdf (Documento final en PDF con el estudio experimental)
 |-- results
 |   |-- plots                        (Gráficos generados: memoria y curvas de tiempo)
 |   |   |-- memoria_screen_name.png
 |   |   |-- memoria_user_id.png
 |   |   |-- tiempo_screen_name.png
 |   |   |-- tiempo_user_id.png
 |   |-- results_memoria.csv          (Datos espaciales crudos de las estructuras)
 |   |-- results_tiempos.csv          (Tiempos crudos de las iteraciones incrementales)
 |   |-- results_tiempos_resumen.csv  (Promedios y desviaciones estándar calculados)
 |-- src
 |   |-- experimentos
 |   |   |-- dataset_loader.h         (Carga modular de los bloques del dataset)
 |   |   |-- experimento.h            (Lógica de las pasadas: 20 runs por configuración)
 |   |-- tablas
 |   |   |-- tabla_hashing_abierto.hpp (Implementación con encadenamiento mediante listas)
 |   |   |-- tabla_hashing_cerrado.hpp (Implementación con Linear, Quadratic y Double Probing)
 |   |   |-- tabla_unordered_map.hpp   (Wrapper para std::unordered_map - STL Baseline)
 |   |-- utils
 |   |   |-- csv_reader.h             (Parser eficiente para el procesamiento del archivo CSV)
 |   |   |-- hash_functions.h         (integerHash de 64-bit y polynomialAccumulation)
 |   |   |-- linked_list.h            (Estructura de soporte lineal para hashing abierto)
 |   |   |-- memory_utils.h           (Herramientas para cálculo de bytes por estructura)
 |   |   |-- prime_utils.h            (Utilidades para búsqueda de tamaños primos - rehashing)
 |   |   |-- timer.h                  (Cronómetro de alta resolución con std::chrono)
 |   |-- main.cpp                     (Punto de entrada y orquestador de experimentos)
 |-- .gitignore
 |-- run_experimentos.sh              (Script de automatización de compilación y corridas)

--------------------------------------------------------------------------------

Metodología y Diseño Experimental

El programa analiza la escalabilidad de las estructuras utilizando un esquema incremental sobre los 180.000 tweets del dataset de las elecciones australianas de 2019.

Esquema de Conteo
Para cada fila del dataset, se evalúa el par de claves bajo la regla solicitada:
if (k en H) { 
    H[k] = H[k] + 1; 
} else {
    H[k] = 1;
}

Parámetros e Implementación de las Estructuras

1. Tabla Hashing Abierto: Manejo de colisiones por encadenamiento lineal. Posee un factor de carga máximo crítico de 1.0, disparando un redimensionamiento dinámico (resize()) al alcanzarse.

2. Tabla Hashing Cerrado: Implementa direccionamiento abierto secuencial y dinámico (factor de carga máximo de 0.7 para evitar degradación extrema). Resuelve colisiones mediante tres modalidades inyectadas en el constructor:
   - Linear Probing: index = (h1(k) + j) % N
   - Quadratic Probing: index = (h1(k) + (j + j^2) / 2) % N
   - Double Hashing: index = (h1(k) + j * h2(k)) % N

3. Funciones Hash Utilizadas:
   - Para user_id (uint64_t): Se descarta la suma de componentes debido a altas tasas de colisión y se adopta un bit mixer robusto de 64 bits para dispersión uniforme.
   - Para user_screen_name (std::string): Acumulación polinomial implementada eficientemente bajo la regla de Horner con base estándar Z = 31.

Rigor del Muestreo
- Cada configuración experimental de tweets (intervalos de 10.000, 20.000, ..., 180.000) se repite 20 veces consecutivas.
- Se reportan de manera obligatoria la media aritmética y la desviación estándar de los tiempos de ejecución para asegurar la reproducibilidad de los datos empíricos en el informe final.