#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "memory_utils.h"
#include <iostream>
#include <tuple>

// Lista enlazada simple de pares (clave, contador). Se usa como bucket de la
// tabla de hashing abierto (encadenamiento).
template <typename T> class LinkedList {
private:
  // Nodo individual de la lista
  struct Node {
    T key;
    int value;
    Node *next;
    Node(const T &val1, int val2) : key(val1), value(val2), next(nullptr) {}
  };

  Node *head; // Puntero al primer nodo

public:
  LinkedList() : head(nullptr) {}

  // Destructor: libera todos los nodos
  ~LinkedList() { clear(); }

  // Se prohíbe copiar la lista: una copia superficial de `head` haría que dos
  // listas liberaran los mismos nodos (double free). No se necesita copiar.
  LinkedList(const LinkedList &) = delete;
  LinkedList &operator=(const LinkedList &) = delete;

  // true si la lista no tiene nodos
  bool empty() const { return head == nullptr; }

  // Inserta un nuevo par al inicio de la lista (O(1)) y retorna una
  // referencia al valor recién insertado, para que la tabla hash pueda
  // usarlo sin volver a recorrer la lista.
  int &insert(const T &val1, int val2) {
    Node *newNode = new Node(val1, val2);
    newNode->next = head;
    head = newNode;
    return head->value;
  }

  // Imprime la lista completa (solo para depuración)
  void display() const {
    Node *current = head;
    while (current != nullptr) {
      std::cout << "(" << current->key << ", " << current->value << ") -> ";
      current = current->next;
    }
    std::cout << "nullptr" << std::endl;
  }

  // Busca la clave linealmente; retorna puntero a su valor o nullptr si no está
  int *search(const T &key) {
    Node *sig = head;
    while (sig) {
      if (key == sig->key)
        return &(sig->value);
      sig = sig->next;
    }
    return nullptr;
  }

  // Extrae y retorna el primer par de la lista.
  // Precondición: la lista NO debe estar vacía (verificar con empty()).
  std::tuple<T, int> pop() {
    auto val = std::tuple<T, int>(head->key, head->value);
    Node *sig = head->next;
    delete head;
    head = sig;
    return val;
  }

  // Estimación de bytes ocupados por los nodos de esta lista (ver
  // memory_utils.h para el detalle de heapExtraBytes)
  size_t memoryBytes() const {
    size_t total = 0;
    Node *current = head;
    while (current != nullptr) {
      total += sizeof(Node) + heapExtraBytes(current->key);
      current = current->next;
    }
    return total;
  }

  // Libera todos los nodos
  void clear() {
    Node *current = head;
    while (current != nullptr) {
      Node *nextNode = current->next;
      delete current;
      current = nextNode;
    }
    head = nullptr;
  }
};

#endif // LINKED_LIST_H
