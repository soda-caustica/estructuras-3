#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <cstddef>
#include <string>

// ============================================================================
// Estimación del uso de memoria adicional en el heap que ocupa una clave
// más allá de lo que ya cuenta `sizeof(K)`.
//
// - Para tipos "planos" como uint64_t, sizeof(K) ya representa toda la
//   memoria que ocupa la clave (no hay heap asociado): heapExtraBytes = 0.
// - Para std::string, sizeof(std::string) cubre el objeto en sí (puntero,
//   tamaño, capacidad y el búfer de Small String Optimization), pero si el
//   string es más largo que ese búfer interno reserva un buffer aparte en
//   el heap. SSO_CAPACITY = 15 es el umbral típico de libstdc++/libc++ en
//   sistemas de 64 bits (long long, la implementación de GCC usada en este
//   proyecto): strings de hasta 15 caracteres no reservan heap adicional.
//
// Esto es una ESTIMACIÓN (no hay forma 100% portable de medir el heap real
// usado por un std::string), pero es razonable para reportar tamaños en el
// informe (sección 4: tamaño de las estructuras en B/KB/MB).
// ============================================================================

// Caso general: tipos sin memoria adicional en el heap (ej. uint64_t)
template <typename K> inline size_t heapExtraBytes(const K &) { return 0; }

// Especialización para std::string
inline size_t heapExtraBytes(const std::string &s) {
  constexpr size_t SSO_CAPACITY = 15;
  return s.capacity() > SSO_CAPACITY ? (s.capacity() + 1) : 0;
}

#endif // MEMORY_UTILS_H
