#pragma once
#include "complex"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fftw3.h>
#include <qcontainerfwd.h>
#include <vector>
#include <QVector>
#include "ConfigManager.h"

class SignalManager {
public:
  static SignalManager &instance();

  QVector<int16_t> GetSinValue();
  QVector<float> GetFftValue(const QVector<int16_t>&);

private:
  int offset = 0;
};
class FftWrapper {
public:
  void SetData(QVector<int16_t>);
  bool CreateFftPlan();
  void StartFftPlan();
  void Clear();
  QVector<float> GetResult();
  FftWrapper();
  ~FftWrapper();

private:
  std::vector<double> fft_in;
  int FFT_PACKET_SIZE = 0;
  int current_fft_size = 0;
  int fft_rate = 0;
  int sample_rate = 0;
  std::vector<std::complex<double>> fft_out;
  fftw_plan fft_plan = nullptr;
};