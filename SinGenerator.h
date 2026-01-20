#pragma once
#include <QCoreApplication>
#include <qcontainerfwd.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include "SignalConfig.h"

class SinGenerator : public QObject {
  Q_OBJECT
public slots:
  void start();
  void stop();
  void generate();

signals:
  void sinReady(const QVector<int16_t> &data);

private:
  std::atomic_bool running{false};
  QVector<int16_t> sin_generate();
  QVector<int16_t> sin_result;
};