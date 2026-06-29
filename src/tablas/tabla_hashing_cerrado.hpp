#ifndef TABLA_HASHING_CERRADO_H
#define TABLA_HASHING_CERRADO_H

#include <cmath>
#include <functional>
#include <memory>
#include <stdexcept>
#include <tuple>

enum ModoTablaHash { LINEAR, QUADRATIC, DOUBLE };

inline bool isPrime(int num) {
  if (num <= 1) {
    return false;
  }
  for (int i = 2; i <= sqrt(num); i++) {
    if (num % i == 0) {
      return false;
    }
  }
  return true;
}
template <typename K> class TablaHashingCerrado {
private:
  std::tuple<K, int> **array;
  int N;
  int numKeys;
  ModoTablaHash modo;
  const double LOAD_FACTOR = 1;
  std::function<int(K)> hash;
  int q;

public:
  TablaHashingCerrado(std::function<int(K)> hashFunction,
                      int initialCapacity = 11, ModoTablaHash modo = LINEAR)
      : hash(hashFunction), N(initialCapacity), numKeys(0), modo(modo) {
    array = new std::tuple<K, int> *[N]();
    // Usaremos la funcion de hashing secundaria mencionada en clase, buscamos
    // el primo menor a N mas cercano
    if (modo == DOUBLE) {
      q = N - 2;
      if (q % 2 == 0)
        q++;
      while (!isPrime(q)) {
        q -= 2;
      }
    }
  }
  ~TablaHashingCerrado() {
    for (int i = 0; i < N; i++) {
      if (array[i] != nullptr) {
        delete array[i];
      }
    }
    delete[] array;
  }
  int len() { return numKeys; }

  int &operator[](K key) {
    int j = 0;
    int hashBase = hash(key);
    int index = hashBase % N;
    while (array[index] != nullptr) {
      if (std::get<0>(*array[index]) == key) {
        return std::get<1>(*array[index]);
      }
      j++;
      if (j >= N) {
        throw std::runtime_error(
            "Tabla de hash llena o límite de exploración alcanzado.");
      }
      switch (modo) {
      case LINEAR:
        index = (hashBase + j) % N;
        break;
      case QUADRATIC:
        index = (hashBase + (j + j * j) / 2) % N;
        break;
      case DOUBLE:
        index = (hashBase + j * (q - (hashBase % N))) % N;
        break;
      }
    }
    array[index] = new std::tuple<K, int>(key, 0);
    numKeys++;
    return std::get<1>(*array[index]);
  }
};

#endif
