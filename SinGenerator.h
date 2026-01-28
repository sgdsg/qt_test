#pragma once
#include "ComponentInterface.h"
#include <QCoreApplication>
#include <qcontainerfwd.h>
#include <qthread.h>
#include <qtmetamacros.h>

class SinGenerator : ComponentInterface {
  Q_OBJECT
public slots:
  void start();
  void stop();
  void generate();

signals:
  void sinReady(const QVector<int16_t> &data);

private:
  QVector<int16_t> sin_generate();
  QVector<int16_t> sin_result;
};