#include "SinGenerator.h"
#include "SignalManager.h"
#include <cstdint>
#include <qcontainerfwd.h>
#include <qlogging.h>
#include <qthread.h>
void SinGenerator::start() {
  this->moveToThread(thread);
  thread->start();
  if (running.exchange(true))
    return;
  while (running.load()) {
    if (!ConfigManager::instance().isRunning()) {
      qInfo() << "Sin Waiting Config...";
      QThread::msleep(1000);
      continue;
    }
    generate();
  }
}

void SinGenerator::stop() { running.store(false); }

void SinGenerator::generate() {
  QVector<int16_t> sin_data = sin_generate();
  emit sinReady(sin_data);
  QThread::msleep(10);
}

QVector<int16_t> SinGenerator::sin_generate() {
  sin_result = SignalManager::instance().GetSinValue();
  return sin_result;
}