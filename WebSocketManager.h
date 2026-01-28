#pragma once
#include "FFTGenerator.h"
#include "ConfigManager.h"
#include "SinGenerator.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/qwebsocket.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <qcontainerfwd.h>
#include <qtmetamacros.h>
class ClientSession : public QObject {
  Q_OBJECT
public:
  ClientSession(QWebSocket *socket, SinGenerator *sin, FFTGenerator *fft);

private slots:
  void handleMessage(const QString &message);
  void handleDisconnect();
  void handleSinData(QVector<int16_t> data);
  void handleFFTData(QVector<float> data);

private:
  QWebSocket *ws = nullptr;
  SinGenerator *singen = nullptr;
  FFTGenerator *fftgen = nullptr;

  QByteArray packInt16(const QVector<int16_t> &vec) {
    QByteArray bytes;
    bytes.resize(vec.size() * int(sizeof(int16_t)));
    std::memcpy(bytes.data(), vec.constData(), size_t(bytes.size()));
    return bytes;
  }

  QByteArray packfloat(const QVector<float> &vec) {
    QByteArray bytes;
    bytes.resize(vec.size() * int(sizeof(float)));
    std::memcpy(bytes.data(), vec.constData(), size_t(bytes.size()));
    return bytes;
  }
};