#pragma once
#include "FFTGenerator.h"
#include "SinGenerator.h"
#include "WebSocketManager.h"
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/qwebsocketserver.h>
#include <qtmetamacros.h>

class WebSocketServer : public QObject {
  Q_OBJECT
public:
  WebSocketServer(quint16 port);

private:
  QWebSocketServer server;
  SinGenerator *sin = nullptr;
  FFTGenerator *fft = nullptr;
};