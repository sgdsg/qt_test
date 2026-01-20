#include "WebSocketServer.h"
#include <QObject>
#include <QThread>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qobject.h>
#include <qthread.h>
#include <qtimer.h>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  WebSocketServer s(1037);
  return a.exec();
}
