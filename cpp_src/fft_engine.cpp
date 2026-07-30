#include "fft_engine.h"

namespace temene {

const double PI = std::acos(-1.0);

void FftEngine::fft(std::vector<Complex>& a, bool invert) {
  size_t n = a.size();
  if (n <= 1) return;

  for (size_t i = 1, j = 0; i < n; i++) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }

  for (size_t len = 2; len <= n; len <<= 1) {
    double ang = 2 * PI / len * (invert ? -1 : 1);
    Complex wlen(std::cos(ang), std::sin(ang));
    for (size_t i = 0; i < n; i += len) {
      Complex w(1);
      for (size_t j = 0; j < len / 2; j++) {
        Complex u = a[i + j];
        Complex v = a[i + j + len / 2] * w;
        a[i + j] = u + v;
        a[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }

  if (invert) {
    for (Complex& x : a) x /= n;
  }
}

std::vector<double> FftEngine::applyHanningWindow(const std::vector<double>& signal) {
  std::vector<double> res(signal.size());
  size_t n = signal.size();
  for (size_t i = 0; i < n; ++i) {
    double multiplier = 0.5 * (1.0 - std::cos(2.0 * PI * i / (n - 1)));
    res[i] = signal[i] * multiplier;
  }
  return res;
}

std::vector<double> FftEngine::applyHammingWindow(const std::vector<double>& signal) {
  std::vector<double> res(signal.size());
  size_t n = signal.size();
  for (size_t i = 0; i < n; ++i) {
    double multiplier = 0.54 - 0.46 * std::cos(2.0 * PI * i / (n - 1));
    res[i] = signal[i] * multiplier;
  }
  return res;
}

std::vector<double> FftEngine::calculatePowerSpectrum(const std::vector<double>& signal) {
  size_t n = 1;
  while (n < signal.size()) n <<= 1;

  std::vector<Complex> comp(n, 0);
  for (size_t i = 0; i < signal.size(); ++i) comp[i] = signal[i];

  fft(comp, false);

  std::vector<double> spectrum(n / 2);
  for (size_t i = 0; i < n / 2; ++i) {
    spectrum[i] = std::norm(comp[i]);
  }
  return spectrum;
}

} // namespace temene
