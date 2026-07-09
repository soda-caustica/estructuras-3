#ifndef TABLA_UNORDERED_MAP_H
#define TABLA_UNORDERED_MAP_H

#include "../utils/memory_utils.h"
#include <unordered_map>
#include <utility>

// ============================================================================
// Wrapper de std::unordered_map de la STL, usado como estructura de
// REFERENCIA (baseline) en los experimentos.
//
// Expone la misma interfaz que TablaHashingAbierto y TablaHashingCerrado
// (operator[], len(), capacity()), de modo que el harness experimental pueda
// tratar a las tres estructuras de forma uniforme.
//
// std::unordered_map implementa hashing abierto (encadenamiento por buckets)
// y usa std::hash<K> como función hash por defecto; se deja la configuración
// por defecto de la STL precisamente porque su rol es servir de baseline.
// ============================================================================
template <typename K> class TablaUnorderedMap {
private:
  std::unordered_map<K, int> tabla;

public:
  // Cantidad de claves distintas almacenadas
  int len() const { return (int)tabla.size(); }

  // Cantidad de buckets actuales, útil para medir memoria
  int capacity() const { return (int)tabla.bucket_count(); }

  // Acceso estilo diccionario: std::unordered_map ya tiene la semántica del
  // esquema de conteo del enunciado (si la clave no existe, operator[] la
  // inserta con int inicializado en 0 y retorna la referencia).
  int &operator[](const K &key) { return tabla[key]; }

  // Estimación de bytes usados. La STL no expone el layout interno de sus
  // nodos, así que se asume una implementación típica de encadenamiento
  // (ej. libstdc++/libc++): un arreglo de punteros a buckets, más un nodo
  // por clave con un puntero "siguiente" y el par (clave, valor).
  size_t memoryBytes() const {
    const size_t overheadPorNodo = sizeof(void *); // puntero "siguiente"
    size_t total = tabla.bucket_count() * sizeof(void *);
    for (const auto &par : tabla) {
      total += overheadPorNodo + sizeof(std::pair<const K, int>) +
               heapExtraBytes(par.first);
    }
    return total;
  }
};

#endif // TABLA_UNORDERED_MAP_H
