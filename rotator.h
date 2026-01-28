#include "cstdint"
#include <cmath>
#include <complex>
class rotator {
private:
  std::complex<float> d_phase;
  std::complex<float> d_phase_incr;
  unsigned int d_counter;

public:
  rotator() : d_phase(1), d_phase_incr(1), d_counter(0) {}

  std::complex<float> phase() { return d_phase; }
  void set_phase(std::complex<float> phase) {
    d_phase = phase / std::abs(phase);
  }
  void set_phase_incr(std::complex<float> incr) {
    d_phase_incr = incr / std::abs(incr);
  }

  std::complex<float> rotate(std::complex<float> in) {
    d_counter++;

    std::complex<float> z = in * d_phase; // rotate in by phase
    d_phase *= d_phase_incr; // incr our phase (complex mult == add phases)

    if ((d_counter % 512) == 0)
      d_phase /=
          std::abs(d_phase); // Normalize to ensure multiplication is rotation

    return z;
  }
};