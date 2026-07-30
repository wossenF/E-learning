#ifndef FFT_ENGINE_H
#define FFT_ENGINE_H

#include <vector>
#include <complex>
#include <cmath>
#include <iostream>

namespace temene {

using Complex = std::complex<double>;

class FftEngine {
public:
  static void fft(std::vector<Complex>& a, bool invert = false);
  static std::vector<double> calculatePowerSpectrum(const std::vector<double>& signal);

  static std::vector<double> applyHanningWindow(const std::vector<double>& signal);
  static std::vector<double> applyHammingWindow(const std::vector<double>& signal);
};

} // namespace temene

#endif // FFT_ENGINE_H
