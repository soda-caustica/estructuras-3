#ifndef EXPERIMENTO_H
#define EXPERIMENTO_H

#include "../utils/timer.h"
#include "dataset_loader.h"

#include <cstddef>
#include <functional>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

// ============================================================================
// Harness experimental: dado UNA combinación (estructura de datos, clave) ya
// elegida por el llamador, procesa los tweets del dataset en una ÚNICA
// pasada continua y muestrea en los checkpoints (10.000, 20.000, ...,
// 180.000 tweets) el tiempo acumulado desde el inicio de la ejecución. No se
// reconstruye la tabla en cada checkpoint: el enunciado pide "el tiempo que
// toma contar los primeros N tweets", que es naturalmente un tiempo
// acumulado sobre la misma tabla que va creciendo, no el tiempo de N pasadas
// independientes desde cero.
//
// La repetición (20 veces) y el barrido de combinaciones NO ocurren aquí:
// cada ejecución de este programa hace UNA sola pasada (de tiempo o de
// memoria) para la combinación indicada por línea de comandos, y un script
// externo (run_experimentos.sh) es quien invoca el ejecutable 20 veces por
// combinación, redirigiendo la salida a los CSV de resultados. Esto separa
// "medir" (este programa) de "orquestar y repetir" (el script), igual que en
// el ejemplo de heapsort_v1/run_experiments.sh.
// ============================================================================

// Ejecuta una repetición completa: inserta los tweets en `tabla` en orden y
// retorna, para cada checkpoint, el tiempo acumulado (en segundos) desde el
// inicio de la repetición hasta terminar de procesar ese checkpoint.
template <typename K, typename Tabla>
std::vector<double>
ejecutarRepeticion(Tabla &tabla, const std::vector<Tweet> &tweets,
                   const std::function<K(const Tweet &)> &extraerClave,
                   const std::vector<int> &checkpoints) {
  std::vector<double> tiempos;
  tiempos.reserve(checkpoints.size());

  Timer timer;
  size_t siguienteCheckpoint = 0;
  int limite = checkpoints.back();

  for (int i = 0; i < limite && siguienteCheckpoint < checkpoints.size();
       i++) {
    tabla[extraerClave(tweets[(size_t)i])]++;
    if (i + 1 == checkpoints[siguienteCheckpoint]) {
      tiempos.push_back(timer.elapsedSeconds());
      siguienteCheckpoint++;
    }
  }
  return tiempos;
}

// Igual que ejecutarRepeticion, pero en vez de tiempo registra el tamaño en
// bytes de la tabla (memoryBytes()) en cada checkpoint.
template <typename K, typename Tabla>
std::vector<size_t>
medirMemoriaPorCheckpoint(Tabla &tabla, const std::vector<Tweet> &tweets,
                          const std::function<K(const Tweet &)> &extraerClave,
                          const std::vector<int> &checkpoints) {
  std::vector<size_t> memorias;
  memorias.reserve(checkpoints.size());

  size_t siguienteCheckpoint = 0;
  int limite = checkpoints.back();

  for (int i = 0; i < limite && siguienteCheckpoint < checkpoints.size();
       i++) {
    tabla[extraerClave(tweets[(size_t)i])]++;
    if (i + 1 == checkpoints[siguienteCheckpoint]) {
      memorias.push_back(tabla.memoryBytes());
      siguienteCheckpoint++;
    }
  }
  return memorias;
}

// --- Exportación a CSV -------------------------------------------------------
// El encabezado de cada CSV lo escribe run_experimentos.sh una sola vez (con
// `echo`); este programa solo imprime filas de datos por stdout, para que el
// script pueda acumularlas con `>>` en cada invocación.
//
// Separador ';' (no ','): así lo especifica el enunciado ("Recomendaciones
// para realizar un estudio experimental"), con el formato
// numero_experimento;dataset;estructura_de_datos;cantidad_consultas;tiempo_ejecucion.
// Se agrega una columna "clave" (no está en el formato base) porque el
// enunciado exige reportar cada medición separada por user_id y por
// user_screen_name, y el formato dado no distingue esa dimensión.

inline void escribirFilaTiempos(std::ostream &out, int numeroExperimento,
                                const std::string &dataset,
                                const std::string &estructura,
                                const std::string &clave, int cantidadConsultas,
                                double tiempoEjecucion) {
  out << numeroExperimento << ";" << dataset << ";" << estructura << ";"
      << clave << ";" << cantidadConsultas << ";" << std::fixed
      << std::setprecision(9) << tiempoEjecucion << "\n";
}

inline void escribirFilaMemoria(std::ostream &out, const std::string &dataset,
                                const std::string &estructura,
                                const std::string &clave, int cantidadConsultas,
                                size_t memoriaBytes) {
  out << dataset << ";" << estructura << ";" << clave << ";"
      << cantidadConsultas << ";" << memoriaBytes << "\n";
}

#endif // EXPERIMENTO_H
