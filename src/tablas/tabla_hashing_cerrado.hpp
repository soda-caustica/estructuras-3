#ifndef TABLA_HASHING_CERRADO_H
#define TABLA_HASHING_CERRADO_H

#include "../utils/memory_utils.h"
#include "../utils/prime_utils.h"
#include <cstddef>
#include <functional>
#include <stdexcept>

// Estrategia de resolución de colisiones de la tabla cerrada
enum ModoTablaHash { LINEAR, QUADRATIC, DOUBLE };

// ============================================================================
// Tabla hash con HASHING CERRADO (direccionamiento abierto / open addressing).
//
// Todos los pares (clave, contador) viven dentro del mismo arreglo de celdas.
// Ante una colisión se exploran otras celdas según el modo:
//   - LINEAR    : index = (h1 + j) % N
//   - QUADRATIC : index = (h1 + j^2) % N
//   - DOUBLE    : index = (h1 + j * paso) % N, con paso = q - (h2 % q)
//     donde h2 es una SEGUNDA función hash (requisito del enunciado) y
//     q es el mayor primo menor que N. Así paso queda en [1, q] y, como N es
//     primo, paso es coprimo con N y la exploración recorre toda la tabla.
//
// La capacidad N se mantiene siempre prima y el factor de carga se limita a
// MAX_LOAD_FACTOR = 0.5. Esto garantiza que la exploración cuadrática
// siempre encuentra una celda libre (con N primo y tabla a lo más medio
// llena, los primeros ceil(N/2) saltos cuadráticos visitan celdas distintas;
// ver Weiss, "Data Structures and Algorithm Analysis in C++", teorema del
// quadratic probing) y evita la degradación por clustering de linear probing.
//
// Costos: inserción / búsqueda O(1) esperado mientras lambda <= 0.5,
// O(n) peor caso. Rehash O(n) amortizado al superar el factor de carga.
// ============================================================================
template <typename K> class TablaHashingCerrado {
private:
  // Celda del arreglo. Se almacena por valor (no punteros): evita una
  // asignación de heap por clave y mejora la localidad de caché.
  struct Celda {
    K key;
    int value;
    bool ocupada;
    Celda() : key(), value(0), ocupada(false) {}
  };

  Celda *array;   // arreglo de celdas
  int N;          // capacidad actual (primo)
  int numKeys;    // cantidad de claves distintas almacenadas
  ModoTablaHash modo;
  const double MAX_LOAD_FACTOR = 0.5;
  std::function<size_t(const K &)> hash;  // función hash principal (h1)
  std::function<size_t(const K &)> hash2; // función hash secundaria (h2),
                                          // usada solo en modo DOUBLE
  int q; // mayor primo < N, para el paso del double hashing

  // Explora la tabla y retorna el índice donde está la clave, o el índice
  // de la primera celda libre de su secuencia de exploración si no está.
  int findSlot(const K &key) const {
    size_t h1 = hash(key);
    // Paso del double hashing: en [1, q], nunca 0 (h2 % q está en [0, q-1]).
    size_t paso = (modo == DOUBLE) ? (size_t)q - (hash2(key) % (size_t)q) : 0;

    int index = (int)(h1 % (size_t)N);
    size_t j = 0;
    while (array[index].ocupada) {
      if (array[index].key == key) {
        return index;
      }
      j++;
      // Con lambda <= 0.5 y N primo esto no debería alcanzarse jamás;
      // es una red de seguridad ante un uso incorrecto de la clase.
      if (j >= (size_t)N) {
        throw std::runtime_error(
            "Tabla de hash llena o límite de exploración alcanzado.");
      }
      switch (modo) {
      case LINEAR:
        index = (int)((h1 + j) % (size_t)N);
        break;
      case QUADRATIC:
        index = (int)((h1 + j * j) % (size_t)N);
        break;
      case DOUBLE:
        index = (int)((h1 + j * paso) % (size_t)N);
        break;
      }
    }
    return index;
  }

  // Crece la capacidad al siguiente primo mayor que 2N, recalcula q y
  // reinserta todas las claves (sus índices dependen de N).
  void rehash() {
    int oldN = N;
    Celda *oldArray = array;

    N = nextPrime(2 * oldN);
    q = prevPrime(N - 1);
    array = new Celda[N];

    for (int i = 0; i < oldN; i++) {
      if (oldArray[i].ocupada) {
        int idx = findSlot(oldArray[i].key);
        array[idx] = oldArray[i];
      }
    }
    delete[] oldArray;
  }

public:
  // hashFunction2 solo es necesaria en modo DOUBLE (segunda función hash
  // que exige el enunciado); en los otros modos puede omitirse.
  TablaHashingCerrado(std::function<size_t(const K &)> hashFunction,
                      int initialCapacity = 11, ModoTablaHash modo = LINEAR,
                      std::function<size_t(const K &)> hashFunction2 = nullptr)
      : N(initialCapacity), numKeys(0), modo(modo), hash(hashFunction),
        hash2(hashFunction2), q(prevPrime(initialCapacity - 1)) {
    if (modo == DOUBLE && !hash2) {
      throw std::invalid_argument(
          "El modo DOUBLE requiere una segunda función hash.");
    }
    array = new Celda[N];
  }

  ~TablaHashingCerrado() { delete[] array; }

  // Se prohíbe copiar la tabla (evita double free del arreglo de celdas).
  TablaHashingCerrado(const TablaHashingCerrado &) = delete;
  TablaHashingCerrado &operator=(const TablaHashingCerrado &) = delete;

  // Cantidad de claves distintas almacenadas
  int len() const { return numKeys; }

  // Capacidad actual (cantidad de celdas), útil para medir memoria
  int capacity() const { return N; }

  // Estimación de bytes usados: el arreglo de celdas (todas, ocupadas o no,
  // ya que se reserva de una vez con `new Celda[N]`) más el heap adicional
  // de las claves ocupadas (relevante solo para K = std::string).
  size_t memoryBytes() const {
    size_t total = (size_t)N * sizeof(Celda);
    for (int i = 0; i < N; i++) {
      if (array[i].ocupada) {
        total += heapExtraBytes(array[i].key);
      }
    }
    return total;
  }

  // Acceso estilo diccionario: si la clave no existe, se inserta con valor 0
  // y se retorna una referencia a él (esquema de conteo del enunciado).
  // OJO: un rehash invalida las referencias retornadas anteriormente.
  int &operator[](const K &key) {
    int index = findSlot(key);
    if (array[index].ocupada) {
      return array[index].value;
    }
    // Clave nueva: rehashear ANTES de insertar si se superaría el factor
    // de carga máximo, y volver a buscar la celda con la nueva capacidad.
    if (numKeys + 1 > N * MAX_LOAD_FACTOR) {
      rehash();
      index = findSlot(key);
    }
    array[index].ocupada = true;
    array[index].key = key;
    array[index].value = 0;
    numKeys++;
    return array[index].value;
  }
};

#endif // TABLA_HASHING_CERRADO_H
