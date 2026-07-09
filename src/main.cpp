#include "experimentos/dataset_loader.h"
#include "experimentos/experimento.h"
#include "tablas/tabla_hashing_abierto.hpp"
#include "tablas/tabla_hashing_cerrado.hpp"
#include "tablas/tabla_unordered_map.hpp"
#include "utils/hash_functions.h"

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// Ejecutable de UNA sola medición, parametrizado por línea de comandos:
//
//   ./experimento <estructura> <clave> tiempo <numero_experimento>
//   ./experimento <estructura> <clave> memoria
//
// <estructura>: hash_abierto | hash_cerrado_linear | hash_cerrado_quadratic
//               | hash_cerrado_double | unordered_map
// <clave>:      user_id | screen_name
//
// Cada invocación carga el dataset, procesa los tweets en una sola pasada
// para la combinación pedida, e imprime a stdout las filas CSV de esa
// pasada (sin encabezado). La repetición 20 veces y el barrido de las 10
// combinaciones los hace run_experimentos.sh, invocando este ejecutable
// muchas veces y redirigiendo la salida — la misma idea que
// heapsort_v1/heapsort_v2 + run_experiments.sh.
// ============================================================================

const std::string RUTA_DATASET = "src/dataset/auspol2019.csv";
const std::string NOMBRE_DATASET = "auspol2019";

std::vector<int> generarCheckpoints() {
  std::vector<int> checkpoints;
  for (int n = 10000; n <= 180000; n += 10000) {
    checkpoints.push_back(n);
  }
  return checkpoints;
}

// Construye la tabla pedida por nombre para clave user_id (uint64_t) y
// ejecuta la medición de tiempo o memoria sobre ella, imprimiendo a stdout.
void ejecutarUserId(const std::string &estructura, const std::string &modo,
                    int numeroExperimento, const std::vector<Tweet> &tweets,
                    const std::vector<int> &checkpoints) {
  std::function<uint64_t(const Tweet &)> extraer = [](const Tweet &t) {
    return t.userId;
  };

  auto medir = [&](auto &tabla) {
    if (modo == "tiempo") {
      std::vector<double> tiempos =
          ejecutarRepeticion<uint64_t>(tabla, tweets, extraer, checkpoints);
      for (size_t c = 0; c < checkpoints.size(); c++) {
        escribirFilaTiempos(std::cout, numeroExperimento, NOMBRE_DATASET,
                           estructura, "user_id", checkpoints[c], tiempos[c]);
      }
    } else {
      std::vector<size_t> memorias = medirMemoriaPorCheckpoint<uint64_t>(
          tabla, tweets, extraer, checkpoints);
      for (size_t c = 0; c < checkpoints.size(); c++) {
        escribirFilaMemoria(std::cout, NOMBRE_DATASET, estructura, "user_id",
                           checkpoints[c], memorias[c]);
      }
    }
  };

  if (estructura == "hash_abierto") {
    auto tabla = TablaHashingAbierto<uint64_t>(hashUserId, 11);
    medir(tabla);
  } else if (estructura == "hash_cerrado_linear") {
    auto tabla = TablaHashingCerrado<uint64_t>(hashUserId, 11, LINEAR);
    medir(tabla);
  } else if (estructura == "hash_cerrado_quadratic") {
    auto tabla = TablaHashingCerrado<uint64_t>(hashUserId, 11, QUADRATIC);
    medir(tabla);
  } else if (estructura == "hash_cerrado_double") {
    auto tabla =
        TablaHashingCerrado<uint64_t>(hashUserId, 11, DOUBLE, hashUserId2);
    medir(tabla);
  } else if (estructura == "unordered_map") {
    auto tabla = TablaUnorderedMap<uint64_t>();
    medir(tabla);
  } else {
    throw std::runtime_error("Estructura desconocida: " + estructura);
  }
}

// Igual que ejecutarUserId, pero para clave screen_name (std::string).
void ejecutarScreenName(const std::string &estructura, const std::string &modo,
                        int numeroExperimento, const std::vector<Tweet> &tweets,
                        const std::vector<int> &checkpoints) {
  std::function<std::string(const Tweet &)> extraer = [](const Tweet &t) {
    return t.userScreenName;
  };

  auto medir = [&](auto &tabla) {
    if (modo == "tiempo") {
      std::vector<double> tiempos =
          ejecutarRepeticion<std::string>(tabla, tweets, extraer, checkpoints);
      for (size_t c = 0; c < checkpoints.size(); c++) {
        escribirFilaTiempos(std::cout, numeroExperimento, NOMBRE_DATASET,
                           estructura, "screen_name", checkpoints[c],
                           tiempos[c]);
      }
    } else {
      std::vector<size_t> memorias = medirMemoriaPorCheckpoint<std::string>(
          tabla, tweets, extraer, checkpoints);
      for (size_t c = 0; c < checkpoints.size(); c++) {
        escribirFilaMemoria(std::cout, NOMBRE_DATASET, estructura,
                           "screen_name", checkpoints[c], memorias[c]);
      }
    }
  };

  if (estructura == "hash_abierto") {
    auto tabla = TablaHashingAbierto<std::string>(hashScreenName, 11);
    medir(tabla);
  } else if (estructura == "hash_cerrado_linear") {
    auto tabla = TablaHashingCerrado<std::string>(hashScreenName, 11, LINEAR);
    medir(tabla);
  } else if (estructura == "hash_cerrado_quadratic") {
    auto tabla =
        TablaHashingCerrado<std::string>(hashScreenName, 11, QUADRATIC);
    medir(tabla);
  } else if (estructura == "hash_cerrado_double") {
    auto tabla = TablaHashingCerrado<std::string>(hashScreenName, 11, DOUBLE,
                                                  hashScreenName2);
    medir(tabla);
  } else if (estructura == "unordered_map") {
    auto tabla = TablaUnorderedMap<std::string>();
    medir(tabla);
  } else {
    throw std::runtime_error("Estructura desconocida: " + estructura);
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "Uso: " << argv[0]
              << " <estructura> <clave> tiempo <numero_experimento>\n"
              << "  o: " << argv[0] << " <estructura> <clave> memoria\n";
    return 1;
  }

  std::string estructura = argv[1];
  std::string clave = argv[2];
  std::string modo = argv[3];

  if (modo != "tiempo" && modo != "memoria") {
    std::cerr << "Modo invalido (usar 'tiempo' o 'memoria'): " << modo
              << "\n";
    return 1;
  }
  int numeroExperimento = 0;
  if (modo == "tiempo") {
    if (argc < 5) {
      std::cerr << "Falta <numero_experimento> para el modo 'tiempo'\n";
      return 1;
    }
    numeroExperimento = std::atoi(argv[4]);
  }

  std::vector<Tweet> tweets = cargarTweets(RUTA_DATASET);
  std::vector<int> checkpoints = generarCheckpoints();
  if (tweets.size() < (size_t)checkpoints.back()) {
    throw std::runtime_error(
        "El dataset tiene menos tweets que el último checkpoint requerido.");
  }

  if (clave == "user_id") {
    ejecutarUserId(estructura, modo, numeroExperimento, tweets, checkpoints);
  } else if (clave == "screen_name") {
    ejecutarScreenName(estructura, modo, numeroExperimento, tweets,
                       checkpoints);
  } else {
    std::cerr << "Clave invalida (usar 'user_id' o 'screen_name'): " << clave
              << "\n";
    return 1;
  }

  return 0;
}
