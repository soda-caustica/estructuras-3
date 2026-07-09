#ifndef CSV_READER_H
#define CSV_READER_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// Lector de CSV compatible con RFC 4180: soporta campos entre comillas dobles
// que contienen comas, saltos de línea o comillas escapadas (""). Es
// necesario porque el dataset de tweets tiene campos (ej. full_text,
// user_description) con saltos de línea internos: el archivo tiene 377.704
// líneas físicas pero solo 183.370 filas de datos reales.
//
// Se parsea desde un std::string con todo el archivo en memoria (en vez de
// carácter a carácter desde un ifstream) porque es notablemente más rápido
// para un archivo de ~70 MB y porque el parseo del CSV se hace UNA sola vez,
// separado de las mediciones de tiempo de las tablas hash.
// ============================================================================

// Lee el archivo completo en un string. Lanza std::runtime_error si no se
// pudo abrir.
inline std::string leerArchivoCompleto(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("No se pudo abrir el archivo: " + path);
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

// Parsea la fila CSV que comienza en contenido[pos], respetando comillas.
// Avanza pos hasta el inicio de la siguiente fila (o hasta el final del
// string) y deja los valores de cada columna en `campos`.
// Retorna false si no quedaba ninguna fila por leer (pos ya estaba al final).
inline bool leerFilaCSV(const std::string &contenido, size_t &pos,
                        std::vector<std::string> &campos) {
  const size_t n = contenido.size();
  if (pos >= n) {
    return false;
  }

  campos.clear();
  std::string campoActual;
  bool dentroDeComillas = false;

  while (pos < n) {
    char ch = contenido[pos];

    if (dentroDeComillas) {
      if (ch == '"') {
        if (pos + 1 < n && contenido[pos + 1] == '"') {
          campoActual += '"'; // comilla escapada ("")
          pos++;
        } else {
          dentroDeComillas = false; // cierre de comillas
        }
      } else {
        campoActual += ch;
      }
    } else {
      if (ch == '"') {
        dentroDeComillas = true;
      } else if (ch == ',') {
        campos.push_back(campoActual);
        campoActual.clear();
      } else if (ch == '\r') {
        // ignorado: se procesa junto con el '\n' que le sigue
      } else if (ch == '\n') {
        pos++;
        campos.push_back(campoActual);
        return true;
      } else {
        campoActual += ch;
      }
    }
    pos++;
  }
  // Última fila del archivo sin salto de línea final
  campos.push_back(campoActual);
  return true;
}

#endif // CSV_READER_H
