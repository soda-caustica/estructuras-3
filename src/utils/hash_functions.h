#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include <cstddef>
#include <cstdint>
#include <string>

// ============================================================================
// Funciones hash para las claves del dataset:
//   - user_id          : entero de 64 bits (los IDs de Twitter superan 2^32)
//   - user_screen_name : string
//
// Para double hashing se necesita una SEGUNDA función hash por tipo de clave,
// por eso hay 4 funciones "con nombre" al final de este archivo:
//   hashUserId / hashUserId2  y  hashScreenName / hashScreenName2
//
// Todas retornan size_t (sin signo): así el módulo % N de las tablas nunca
// produce índices negativos, cosa que sí puede pasar con int por overflow.
// ============================================================================

// Base 31: estándar para acumulación polinomial de strings (es la base que
// usa String.hashCode() de Java; ver Goodrich & Tamassia, "Data Structures
// and Algorithms", cap. de tablas hash). constexpr en vez de #define para
// no contaminar el espacio de nombres global con un macro.
constexpr unsigned int BASE_POLINOMIAL = 31;

// --- Funciones base ---------------------------------------------------------

// Component sum: divide un entero de 64 bits en 8 bytes y suma sus valores
// (Goodrich & Tamassia). Es intencionalmente simple: su rango es pequeño
// (máx 8*255 = 2040), por lo que se usa solo como hash SECUNDARIO de user_id
// para double hashing, no como hash principal.
inline unsigned int componentSum(uint64_t i) {
  return (unsigned char)(i) +
         (unsigned char)(i >> 8) +
         (unsigned char)(i >> 16) +
         (unsigned char)(i >> 24) +
         (unsigned char)(i >> 32) +
         (unsigned char)(i >> 40) +
         (unsigned char)(i >> 48) +
         (unsigned char)(i >> 56);
}

// Acumulación polinomial evaluada con la regla de Horner:
//   h = s[0]*Z^(n-1) + s[1]*Z^(n-2) + ... + s[n-1]
// El overflow de unsigned int está bien definido en C++ (aritmética mod 2^32),
// así que el desborde actúa como reducción natural del polinomio.
inline unsigned int polynomialAccumulation(const std::string &s) {
  unsigned int h = 0;
  for (unsigned char c : s) {
    h = c + h * BASE_POLINOMIAL;
  }
  return h;
}

// Mezclador de bits multiplicativo para enteros de 64 bits. Corresponde al
// finalizador de SplitMix64 (Steele, Lea & Flood, "Fast Splittable
// Pseudorandom Number Generators", OOPSLA 2014). Difunde todos los bits de
// la entrada, de modo que user_ids consecutivos o similares quedan
// distribuidos uniformemente en la tabla.
inline uint64_t splitMix64(uint64_t x) {
  x ^= x >> 30;
  x *= 0xBF58476D1CE4E5B9ULL;
  x ^= x >> 27;
  x *= 0x94D049BB133111EBULL;
  x ^= x >> 31;
  return x;
}

// FNV-1a de 64 bits (Fowler-Noll-Vo, variante 1a): hash clásico de strings,
// procesa byte a byte con XOR + multiplicación por el primo FNV.
// Referencia: http://www.isthe.com/chongo/tech/comp/fnv/
inline uint64_t fnv1a(const std::string &s) {
  uint64_t h = 14695981039346656037ULL; // offset basis FNV-64
  for (unsigned char c : s) {
    h ^= c;
    h *= 1099511628211ULL; // primo FNV-64
  }
  return h;
}

// --- Funciones con nombre usadas por los experimentos -----------------------

// Hash principal para user_id.
inline size_t hashUserId(const uint64_t &id) {
  return (size_t)splitMix64(id);
}

// Hash secundario para user_id (solo para double hashing).
inline size_t hashUserId2(const uint64_t &id) {
  return (size_t)componentSum(id);
}

// Hash principal para user_screen_name.
inline size_t hashScreenName(const std::string &s) {
  return (size_t)polynomialAccumulation(s);
}

// Hash secundario para user_screen_name (solo para double hashing).
inline size_t hashScreenName2(const std::string &s) {
  return (size_t)fnv1a(s);
}

#endif // HASH_FUNCTIONS_H
