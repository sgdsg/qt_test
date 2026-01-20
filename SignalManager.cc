#include "SignalManager.h"
#include <cstdint>
#include <qcontainerfwd.h>

SignalManager &SignalManager::instance() {
  static SignalManager instance;
  return instance;
}

QVector<int16_t> SignalManager::GetSinValue() {
  auto cfg = ConfigManager::instance().get();
  int amplitude = cfg.amplitude;
  int freq = cfg.frequency;
  int sample_rate = cfg.sample_rate_hz;
  QVector<int16_t> data(sample_rate);
  for (int i = 0; i < sample_rate; i++) {
    int index = (i + offset) % sample_rate;
    double sin_val = std::sin(
        2.0 * M_PI * freq *
        (static_cast<double>(index) / static_cast<double>(sample_rate)));
    data[i] = static_cast<int16_t>(amplitude * sin_val);
  }

  offset += 25;

  int16_t last_sample = data[sample_rate - 1];
  std::memmove(data.data(), data.data() + 1, (sample_rate - 1) * sizeof(int16_t));
  data[sample_rate - 1] = last_sample;

  return data;
}

FftWrapper::FftWrapper() {}

FftWrapper::~FftWrapper() {
  if (fft_plan != nullptr) {
    fftw_destroy_plan(fft_plan);
  }
}

void FftWrapper::SetData(QVector<int16_t> data) {   
  fft_rate = ConfigManager::instance().get().fft_rate;
  current_fft_size = fft_rate;
  sample_rate = ConfigManager::instance().get().sample_rate_hz;
  int old_size = fft_in.size();
  FFT_PACKET_SIZE += data.size();
  fft_in.resize(FFT_PACKET_SIZE);
  for (int i = 0; i < data.size(); i++) {
    fft_in[old_size + i] = static_cast<double>(data[i]);
  }
  if ((fft_rate - FFT_PACKET_SIZE) <= sample_rate) {
    fft_in.resize(fft_rate, 0.0);
    FFT_PACKET_SIZE = fft_rate;
  }
}

bool FftWrapper::CreateFftPlan() {
  fft_rate = ConfigManager::instance().get().fft_rate;
  if (fft_rate <= 0) {
    return false;
  }

  if (FFT_PACKET_SIZE < fft_rate) {
    return false;
  }

  if (fft_rate != current_fft_size) {
    current_fft_size = fft_rate;
  }
  if (fft_plan) {
    fftw_destroy_plan(fft_plan);
    fft_plan = nullptr;
  }
  fft_out.resize(current_fft_size / 2 + 1);

  fft_plan = fftw_plan_dft_r2c_1d(
      current_fft_size, fft_in.data(),
      reinterpret_cast<fftw_complex *>(fft_out.data()), FFTW_ESTIMATE);

  return true;
}

void FftWrapper::StartFftPlan() { fftw_execute(fft_plan); }

void FftWrapper::Clear() {
  FFT_PACKET_SIZE = 0;
  fft_in.clear();
}

QVector<float> FftWrapper::GetResult() {
  QVector<float> fft_data(current_fft_size/2+1);
  for (int i = 0; i < current_fft_size / 2 + 1; i++) {
    float mag = std::abs(fft_out[i]);
    fft_data[i] = mag / current_fft_size;
  }
  return fft_data;
}

QVector<float> SignalManager::GetFftValue(const QVector<int16_t>& time_data) {
  auto cfg = ConfigManager::instance().get();
  if (cfg.fft_enable) {

    static FftWrapper fft;

    fft.SetData(time_data);

    if (fft.CreateFftPlan()) {
      fft.StartFftPlan();
      auto result = fft.GetResult();
      fft.Clear();
      return result;
    }
  }
  return {};
}
