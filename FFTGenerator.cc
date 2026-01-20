#include "FFTGenerator.h"
#include "SignalManager.h"
#include <cstdint>
#include <qcontainerfwd.h>
#include <qlogging.h>
#include <qthread.h>
void FFTGenerator::start() { running.store(true); }

void FFTGenerator::stop() { running.store(false); }

void FFTGenerator::handleSinData(const QVector<int16_t> &data) {
  if (!running.load())
    return;
  if (!ConfigManager::instance().isRunning())
    return;
  fft_result = SignalManager::instance().GetFftValue(data);
  emit fftReady(fft_result);
}