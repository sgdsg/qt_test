#include "WebSocketServer.h"
#include "FFTGenerator.h"
#include "SinGenerator.h"
#include <QtNetwork/qhostaddress.h>
#include <QtWebSockets/qwebsocketserver.h>
#include <qcoreapplication.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qthread.h>
WebSocketServer::WebSocketServer(quint16 port)
    : server(QStringLiteral("QtWsServer"), QWebSocketServer::NonSecureMode) {
  server.listen(QHostAddress::Any, port);
  qInfo() << "WS listening on port" << server.serverPort();
  QThread *sinThread = new QThread(this);
  sin = new SinGenerator;
  sin->moveToThread(sinThread);
  connect(sinThread, &QThread::started, sin, &SinGenerator::start);
  connect(sinThread, &QThread::finished, sin, &SinGenerator::stop);
  connect(sinThread, &QThread::finished, sin, &QObject::deleteLater);
  sinThread->start();

  QThread *fftThread = new QThread(this);
  fft = new FFTGenerator;
  fft->moveToThread(fftThread);
  connect(sin, &SinGenerator::sinReady, fft, &FFTGenerator::handleSinData, Qt::QueuedConnection);
  connect(fftThread, &QThread::started, fft, &FFTGenerator::start);
  connect(fftThread, &QThread::finished, fft, &FFTGenerator::stop);
  connect(fftThread, &QThread::finished, fft, &QObject::deleteLater);
  fftThread->start();

  connect(&server, &QWebSocketServer::newConnection, this, [this] {
    QWebSocket *ws = server.nextPendingConnection();
    qInfo() << "Client Connected";
    new ClientSession(ws, sin, fft);
  });
}