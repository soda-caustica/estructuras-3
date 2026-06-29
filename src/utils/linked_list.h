#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <functional>
#include <iostream>
#include <tuple>

// Hecho con IA ya que no se evaluan linked lists
template <typename T> class LinkedList {
private:
  // Struct representing a single node in the list
  struct Node {
    T key;
    int value;
    Node *next;
    Node(const T &val1, int val2) : key(val1), next(nullptr), value(val2) {}
  };

  Node *head; // Pointer to the first node

public:
  // Constructor
  LinkedList() : head(nullptr) {}

  // Destructor to prevent memory leaks
  ~LinkedList() { clear(); }

  // Insert a new element at the beginning of the list
  void insert(const T &val1, int val2) {
    Node *newNode = new Node(val1, val2);
    newNode->next = head;
    head = newNode;
  }

  // Print the entire list
  void display() const {
    Node *current = head;
    while (current != nullptr) {
      std::cout << current->data << " -> ";
      current = current->next;
    }
    std::cout << "nullptr" << std::endl;
  }

  /// Toma un predicado y busca en la lista linealmente
  int *search(T key) {
    auto sig = head;
    while (sig) {
      if (key == sig->key)
        return &(sig->value);
      sig = sig->next;
    }
    return nullptr;
  }

  std::tuple<T, int> pop() {
    auto val = std::tuple<T, int>(head->key, head->value);
    auto sig = head->next;
    delete head;
    head = sig;
    return val;
  }

  // Free all allocated memory
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
