#pragma once
#include <QCoreApplication>
#include <qcontainerfwd.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include "SignalConfig.h"

class FFTGenerator : public QObject {
  Q_OBJECT
public slots:
  void start();
  void stop();
  void handleSinData(const QVector<int16_t>& data);

signals:
  void fftReady(const QVector<float> &data);

private:
  std::atomic_bool running{false};
  QVector<float> fft_result;
};