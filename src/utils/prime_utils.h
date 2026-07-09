#ifndef PRIME_UTILS_H
#define PRIME_UTILS_H

// Utilidades de números primos compartidas por las tablas hash.
// Se usan para elegir capacidades primas, lo que mejora la dispersión
// del operador módulo y es requisito para que double hashing recorra
// todas las celdas (paso coprimo con N).

// Retorna true si num es primo. Prueba divisores hasta sqrt(num)
// usando i*i <= num para evitar problemas de precisión de punto flotante.
inline bool isPrime(int num) {
  if (num < 2) {
    return false;
  }
  for (long i = 2; i * i <= num; i++) {
    if (num % i == 0) {
      return false;
    }
  }
  return true;
}

// Retorna el menor primo mayor o igual a n (para crecer la tabla al rehashear).
inline int nextPrime(int n) {
  if (n < 2) {
    return 2;
  }
  while (!isPrime(n)) {
    n++;
  }
  return n;
}

// Retorna el mayor primo menor o igual a n (para el primo q < N de double hashing).
inline int prevPrime(int n) {
  while (n >= 2 && !isPrime(n)) {
    n--;
  }
  return n;
}

#endif // PRIME_UTILS_H
