#ifndef WEB_DASHBOARD_MANAGER_H
#define WEB_DASHBOARD_MANAGER_H

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "../config/Config.h"
#include "../config/SystemTypes.h"
#include "../system/StorageManager.h"
#include "../system/TimeManager.h"
#include "../controller/ParkingController.h"
#include "WiFiManager.h"
#include "WebBundle.h"

class WebDashboardManager {
public:
  WebDashboardManager(
    ParkingController &parking,
    StorageManager &storage,
    WiFiManager &wifi
  );

  // Inisialisasi HTTP routes dan WebSocket server
  void inisialisasi();

  // Loop update server HTTP dan WebSocket
  void update(const UIState &state);

  // Broadcast data telemetri real-time ke semua klien browser
  void broadcastTelemetry(const UIState &state);

  // Broadcast log peristiwa ke konsol browser
  void broadcastLog(const char* category, const String &message);

  // Callback event WebSocket
  void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

private:
  WebServer server;
  WebSocketsServer wsServer;

  ParkingController &parking;
  StorageManager &storage;
  WiFiManager &wifi;

  unsigned long waktuBroadcastTerakhir;
  unsigned long waktuMulaiSistem;

  // Nilai cache untuk deteksi perubahan status (push-on-change)
  int prevSlots;
  int prevMaxSlots;
  bool prevGateOpen;
  bool prevCarLeft;
  bool prevCarRight;

  void setupHttpRoutes();
  void handleCommand(const String &jsonStr);
  int extractJsonInt(const String &json, const String &key, int fallback = 0);
  String extractJsonString(const String &json, const String &key, const String &fallback = "");
};

#endif // WEB_DASHBOARD_MANAGER_H

