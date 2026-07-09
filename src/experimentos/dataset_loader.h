#ifndef DATASET_LOADER_H
#define DATASET_LOADER_H

#include "../utils/csv_reader.h"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// Carga del dataset de tweets (src/dataset/auspol2019.csv) para el
// experimento de conteo de tweets por usuario.
//
// Solo se extraen las dos claves que exige el enunciado (user_id y
// user_screen_name): el resto de las columnas (full_text, user_description,
// etc.) no se necesitan para el conteo y no se guardan, para mantener bajo
// el uso de memoria del harness.
//
// Las columnas se ubican por NOMBRE en el encabezado (no por posición fija),
// de modo que el loader no dependa del orden exacto de columnas del CSV.
// ============================================================================

struct Tweet {
  uint64_t userId;
  std::string userScreenName;
};

// Busca el índice de una columna por nombre en el encabezado ya parseado.
// Lanza std::runtime_error si la columna no existe.
inline size_t indiceColumna(const std::vector<std::string> &encabezado,
                           const std::string &nombre) {
  for (size_t i = 0; i < encabezado.size(); i++) {
    if (encabezado[i] == nombre) {
      return i;
    }
  }
  throw std::runtime_error("Columna no encontrada en el CSV: " + nombre);
}

// Carga todos los tweets del CSV en memoria como vector<Tweet>.
// Lanza std::runtime_error si el archivo no se puede abrir, si falta alguna
// columna requerida, o si user_id no es convertible a entero.
inline std::vector<Tweet> cargarTweets(const std::string &path) {
  std::string contenido = leerArchivoCompleto(path);

  size_t pos = 0;
  std::vector<std::string> fila;

  if (!leerFilaCSV(contenido, pos, fila)) {
    throw std::runtime_error("El archivo CSV está vacío: " + path);
  }
  size_t colUserId = indiceColumna(fila, "user_id");
  size_t colScreenName = indiceColumna(fila, "user_screen_name");
  size_t numColumnas = fila.size();

  std::vector<Tweet> tweets;
  tweets.reserve(200000); // reserva aproximada: el dataset tiene ~183.000 filas

  while (leerFilaCSV(contenido, pos, fila)) {
    // Se ignoran silenciosamente líneas en blanco al final del archivo
    // (fila de un solo campo vacío, producto del último '\n' del archivo).
    if (fila.size() == 1 && fila[0].empty()) {
      continue;
    }
    if (fila.size() != numColumnas) {
      throw std::runtime_error(
          "Fila con número de columnas inconsistente en el CSV (se esperaban " +
          std::to_string(numColumnas) + ", se obtuvieron " +
          std::to_string(fila.size()) + ")");
    }
    Tweet t;
    t.userId = std::stoull(fila[colUserId]);
    t.userScreenName = fila[colScreenName];
    tweets.push_back(std::move(t));
  }

  return tweets;
}

#endif // DATASET_LOADER_H
