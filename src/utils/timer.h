#ifndef TIMER_H
#define TIMER_H

#include <chrono>

// Cronómetro simple basado en std::chrono::steady_clock: un reloj monótono
// (nunca retrocede ni se ve afectado por ajustes del reloj del sistema),
// por lo que es la elección correcta para medir duraciones, a diferencia de
// std::chrono::system_clock.
class Timer {
private:
  std::chrono::steady_clock::time_point inicio;

public:
  Timer() { reset(); }

  // Reinicia el punto de partida del cronómetro
  void reset() { inicio = std::chrono::steady_clock::now(); }

  // Segundos transcurridos desde el último reset() (con decimales)
  double elapsedSeconds() const {
    auto ahora = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(ahora - inicio).count();
  }
};

#endif // TIMER_H
