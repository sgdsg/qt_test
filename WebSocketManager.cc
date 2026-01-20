#include "WebSocketManager.h"
#include "FFTGenerator.h"
#include "SignalConfig.h"
#include "SinGenerator.h"
#include <QtWebSockets/qwebsocket.h>
#include <qjsonparseerror.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qstringview.h>
ClientSession::ClientSession(QWebSocket *socket, SinGenerator *sin,
                             FFTGenerator *fft)
    : ws(socket), singen(sin), fftgen(fft) {
  QObject::connect(ws, &QWebSocket::textMessageReceived, this,
                   &ClientSession::handleMessage);
  QObject::connect(ws, &QWebSocket::disconnected, this,
                   &ClientSession::handleDisconnect);
  QObject::connect(sin, &SinGenerator::sinReady, this,
                   &ClientSession::handleSinData, Qt::QueuedConnection);
  QObject::connect(fft, &FFTGenerator::fftReady, this,
                   &ClientSession::handleFFTData, Qt::QueuedConnection);
}

void ClientSession::handleMessage(const QString &message) {
  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
  if (doc.isNull()) {
    ws->sendTextMessage(R"({"error":"invalid json"})");
    return;
  }

  SignalDto config_dto;
  QString error;
  if (!ConfigManager::instance().parse(doc, config_dto, error)) {
    ws->sendTextMessage(QString(R"({"error":"%1"})").arg(error));
    return;
  }

  ConfigManager::instance().update(config_dto);
  ws->sendTextMessage(R"({"status":"config received"})");
}

void ClientSession::handleDisconnect() {
  qInfo() << "Client Disconnected";
  ws->deleteLater();
  deleteLater();
}

void ClientSession::handleSinData(QVector<int16_t> data) {
  if (!ConfigManager::instance().isRunning())
    return;
  QByteArray sending_data;
  sending_data.append(char(0));
  sending_data.append(packInt16(data));
  ws->sendBinaryMessage(sending_data);
}

void ClientSession::handleFFTData(QVector<float> data) {
  if (!ConfigManager::instance().isRunning())
    return;
  QByteArray sending_data;
  sending_data.append(char(1));
  sending_data.append(packfloat(data));
  ws->sendBinaryMessage(sending_data);
}