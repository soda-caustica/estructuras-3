#ifndef TABLA_HASHING_ABIERTO_H
#define TABLA_HASHING_ABIERTO_H

#include "../utils/linked_list.h"
#include <functional>
template <typename K> class TablaHashingAbierto {
private:
  LinkedList<K> *array;
  int N;
  int numKeys;
  const double LOAD_FACTOR = 1;
  std::function<int(K)> hash;

public:
  TablaHashingAbierto(std::function<int(K)> hashFunction,
                      int initialCapacity = 11)
      : hash(hashFunction), N(initialCapacity), numKeys(0) {
    array = new LinkedList<K>[N];
  }
  ~TablaHashingAbierto() { delete[] array; }
  int len() { return numKeys; }
  int &operator[](K key) {
    int index = hash(key) % N;

    int *found = array[index].search(key);

    if (found == nullptr) {
      array[index].insert(key, 0);
      numKeys++;
      found = array[index].search(key);
    }
    return *found;
  }
};

#endif
