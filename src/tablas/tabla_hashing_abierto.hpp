#ifndef TABLA_HASHING_ABIERTO_H
#define TABLA_HASHING_ABIERTO_H

#include "../utils/linked_list.h"
#include "../utils/prime_utils.h"
#include <cstddef>
#include <functional>
#include <tuple>

// ============================================================================
// Tabla hash con HASHING ABIERTO (encadenamiento / separate chaining).
//
// Cada celda del arreglo es una lista enlazada con todos los pares
// (clave, contador) cuyo hash cae en esa celda. Las colisiones se resuelven
// agregando a la lista, por lo que la tabla nunca "se llena".
//
// Costos (con función hash uniforme y factor de carga lambda = n/N):
//   - Inserción / búsqueda: O(1 + lambda) esperado, O(n) peor caso
//     (todas las claves en el mismo bucket).
//   - Cuando lambda supera MAX_LOAD_FACTOR se duplica la capacidad al
//     siguiente primo y se redistribuyen las claves (rehash, O(n) amortizado).
//
// Parámetros por defecto: capacidad inicial 11 (primo), factor de carga
// máximo 1.0 (en promedio una clave por bucket).
// ============================================================================
template <typename K> class TablaHashingAbierto {
private:
  LinkedList<K> *array; // arreglo de buckets (listas enlazadas)
  int N;                // capacidad actual (cantidad de buckets, primo)
  int numKeys;          // cantidad de claves distintas almacenadas
  const double MAX_LOAD_FACTOR = 1.0;
  std::function<size_t(const K &)> hash;

  // Índice del bucket de una clave. El hash es size_t (sin signo), por lo
  // que el módulo siempre queda en [0, N-1].
  int indexOf(const K &key) const { return (int)(hash(key) % (size_t)N); }

  // Duplica la capacidad al siguiente primo y redistribuye todas las claves.
  // Se vacían los buckets viejos con pop() y se reinsertan según el nuevo N.
  void rehash() {
    int oldN = N;
    LinkedList<K> *oldArray = array;

    N = nextPrime(2 * oldN);
    array = new LinkedList<K>[N];

    for (int i = 0; i < oldN; i++) {
      while (!oldArray[i].empty()) {
        std::tuple<K, int> par = oldArray[i].pop();
        array[indexOf(std::get<0>(par))].insert(std::get<0>(par),
                                                std::get<1>(par));
      }
    }
    delete[] oldArray;
  }

public:
  TablaHashingAbierto(std::function<size_t(const K &)> hashFunction,
                      int initialCapacity = 11)
      : N(initialCapacity), numKeys(0), hash(hashFunction) {
    array = new LinkedList<K>[N];
  }

  ~TablaHashingAbierto() { delete[] array; }

  // Se prohíbe copiar la tabla (evita double free del arreglo de buckets).
  TablaHashingAbierto(const TablaHashingAbierto &) = delete;
  TablaHashingAbierto &operator=(const TablaHashingAbierto &) = delete;

  // Cantidad de claves distintas almacenadas
  int len() const { return numKeys; }

  // Capacidad actual (cantidad de buckets), útil para medir memoria
  int capacity() const { return N; }

  // Estimación de bytes usados: el arreglo de buckets (cada LinkedList<K>
  // es solo un puntero a su primer nodo) más los nodos de todas las listas.
  size_t memoryBytes() const {
    size_t total = (size_t)N * sizeof(LinkedList<K>);
    for (int i = 0; i < N; i++) {
      total += array[i].memoryBytes();
    }
    return total;
  }

  // Acceso estilo diccionario: si la clave no existe, se inserta con valor 0
  // y se retorna una referencia a él. Esto implementa directamente el
  // esquema de conteo del enunciado: tabla[k]++ crea la entrada si no está.
  // OJO: un rehash invalida las referencias retornadas anteriormente.
  int &operator[](const K &key) {
    int *found = array[indexOf(key)].search(key);
    if (found != nullptr) {
      return *found;
    }
    // Clave nueva: rehashear ANTES de insertar si se superaría el factor
    // de carga máximo, y recalcular el índice con la nueva capacidad.
    if (numKeys + 1 > N * MAX_LOAD_FACTOR) {
      rehash();
    }
    numKeys++;
    return array[indexOf(key)].insert(key, 0);
  }
};

#endif // TABLA_HASHING_ABIERTO_H
