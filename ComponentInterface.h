#pragma once

#include <atomic>
#include <qobject.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <qvariant.h>
class ComponentInterface : public QObject {
  Q_OBJECT
protected:
  ComponentInterface();
public slots:
  virtual void start() = 0;
  virtual void stop();
  virtual void generate();

signals:
  void dataReady(const QVariant &);

protected:
  QThread *thread;
  std::atomic_bool running{false};
};