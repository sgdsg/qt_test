#pragma once
#include <atomic>
#include <iostream>
#include <json/json.h>
#include <mutex>
#include <qlogging.h>
#include <qobject.h>
#include <thread>
#include <QJsonDocument>
#include <QJsonObject>


struct SignalDto {
  int amplitude;
  int frequency;
  int sample_rate_hz;
  int fft_rate;
  bool fft_enable;
};

class ConfigManager {
public:
  static ConfigManager &instance();

  bool parse(const QJsonDocument& config, SignalDto& out, QString& error) {
    if (!config.isObject()) {
        error = "config must be JSON object";
        return false;
    }

    QJsonObject o = config.object();

    if (!o.contains("amplitude")) { error = "missing amplitude"; return false; }
    if (!o.contains("frequency")) { error = "missing frequency"; return false; }
    if (!o.contains("sample_rate_hz")) { error = "missing sample_rate_hz"; return false; }
    if (!o.contains("fft_rate")) { error = "missing fft_rate"; return false; }
    if (!o.contains("fft_enable")) { error = "missing fft_enable"; return false; }

    out.amplitude      = o.value("amplitude").toInt();
    out.frequency      = o.value("frequency").toInt();
    out.sample_rate_hz = o.value("sample_rate_hz").toInt();
    out.fft_rate       = o.value("fft_rate").toInt();
    out.fft_enable     = o.value("fft_enable").toBool();

    return true;
  }

  void update(const SignalDto &dto) {
    std::lock_guard lock(mutex_);
    config = dto;
    running = true;
    
  };

  SignalDto get() {
    std::lock_guard guard(mutex_);
    return config;
  }

  void stop() { running.store(false); }

  bool isRunning() const { return running.load(); }

private:
  SignalDto config;
  std::atomic_bool running{false};
  ConfigManager() = default;
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(ConfigManager const &) = delete;
  mutable std::mutex mutex_;
};
