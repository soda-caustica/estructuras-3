#!/bin/bash

# Tarea 3 - Estructuras de Datos: harness experimental.
# Compila el ejecutable y lo invoca repetidamente para generar los CSV de
# resultados, siguiendo la misma idea que heapsort_v1/v2 + run_experiments.sh:
# el programa C++ solo mide UNA combinación por invocación e imprime sus
# filas CSV a stdout; este script se encarga de repetir y redirigir.

set -e

EXE=./experimento
REPETICIONES=20
ESTRUCTURAS="hash_abierto hash_cerrado_linear hash_cerrado_quadratic hash_cerrado_double unordered_map"
CLAVES="user_id screen_name"

mkdir -p results

echo "Compilando..."
g++ -std=c++17 -O2 -Wall -Wextra -o experimento src/main.cpp

# --- Tiempos: 20 repeticiones por cada (estructura, clave) -----------------
# Formato pedido por el enunciado (separador ';'), con una columna "clave"
# agregada para distinguir user_id de user_screen_name.
echo "numero_experimento;dataset;estructura_de_datos;clave;cantidad_consultas;tiempo_ejecucion" > results/results_tiempos.csv

for rep in $(seq 1 $REPETICIONES); do
  for estructura in $ESTRUCTURAS; do
    for clave in $CLAVES; do
      echo "[tiempo] repeticion $rep - $estructura - $clave"
      $EXE $estructura $clave tiempo $rep >> results/results_tiempos.csv
    done
  done
done

# --- Memoria: 1 sola pasada por cada (estructura, clave) --------------------
# (el tamano de la tabla en un checkpoint es determinista: no varia entre
# repeticiones, asi que no aporta informacion medirlo 20 veces)
echo "dataset;estructura_de_datos;clave;cantidad_consultas;memoria_bytes" > results/results_memoria.csv

for estructura in $ESTRUCTURAS; do
  for clave in $CLAVES; do
    echo "[memoria] $estructura - $clave"
    $EXE $estructura $clave memoria >> results/results_memoria.csv
  done
done

echo "Listo. Resultados en results/results_tiempos.csv y results/results_memoria.csv"
