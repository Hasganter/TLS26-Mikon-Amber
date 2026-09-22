#include "WebDashboardManager.h"

static WebDashboardManager* s_instance = nullptr;

static void globalWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (s_instance) {
    s_instance->onWebSocketEvent(num, type, payload, length);
  }
}

WebDashboardManager::WebDashboardManager(
  ParkingController &p,
  StorageManager &s,
  WiFiManager &w
) : server(HTTP_PORT),
    wsServer(WS_PORT),
    parking(p),
    storage(s),
    wifi(w),
    waktuBroadcastTerakhir(0),
    waktuMulaiSistem(0),
    prevSlots(-1),
    prevMaxSlots(-1),
    prevGateOpen(false),
    prevCarLeft(false),
    prevCarRight(false) {
  s_instance = this;
}

void WebDashboardManager::inisialisasi() {
  waktuMulaiSistem = millis();

  // 1. Inisialisasi Endpoint HTTP
  setupHttpRoutes();
  server.begin();
  Serial.printf("[WEB] HTTP Server aktif di port %d\r\n", HTTP_PORT);

  // 2. Inisialisasi WebSocket Server
  wsServer.begin();
  wsServer.onEvent(globalWsEvent);
  Serial.printf("[WEB] WebSocket Server aktif di port %d\r\n", WS_PORT);
}

void WebDashboardManager::setupHttpRoutes() {
  // Route Utama: Dashboard Single Page Application
  server.on("/", HTTP_GET, [this]() {
    server.sendHeader("Cache-Control", "public, max-age=3600");
    server.send_P(200, "text/html", PAGE_INDEX_HTML);
  });

  // Route ServiceWorker: sw.js
  server.on("/sw.js", HTTP_GET, [this]() {
    server.sendHeader("Cache-Control", "no-cache");
    server.send_P(200, "application/javascript", PAGE_SERVICE_WORKER_JS);
  });

  // Route Web App Manifest: manifest.json
  server.on("/manifest.json", HTTP_GET, [this]() {
    server.sendHeader("Cache-Control", "public, max-age=86400");
    server.send_P(200, "application/json", PAGE_MANIFEST_JSON);
  });

  // Fallback 404
  server.onNotFound([this]() {
    server.send(404, "text/plain", "404: Not Found");
  });
}

void WebDashboardManager::update(const UIState &state) {
  // Layani request HTTP dan WebSocket
  server.handleClient();
  wsServer.loop();

  unsigned long sekarang = millis();

  // Push-on-change: Deteksi perubahan status sensor, gate, atau slot
  bool carLeft = (state.jarakKiri < AMBANG_DETEKSI_CM);
  bool carRight = (state.jarakKanan < AMBANG_DETEKSI_CM);
  bool gateOpen = parking.isGateTerbuka();

  bool adaPerubahan = (state.slotTersedia != prevSlots) ||
                      (state.kapasitasMaksimal != prevMaxSlots) ||
                      (gateOpen != prevGateOpen) ||
                      (carLeft != prevCarLeft) ||
                      (carRight != prevCarRight);

  // Broadcast jika ada perubahan atau periodic heartbeat 1 detik
  if (adaPerubahan || (sekarang - waktuBroadcastTerakhir >= 1000)) {
    waktuBroadcastTerakhir = sekarang;
    prevSlots = state.slotTersedia;
    prevMaxSlots = state.kapasitasMaksimal;
    prevGateOpen = gateOpen;
    prevCarLeft = carLeft;
    prevCarRight = carRight;

    broadcastTelemetry(state);
  }
}

void WebDashboardManager::broadcastTelemetry(const UIState &state) {
  String gateStr = "CLOSED";
  if (parking.isGateTerkunci()) {
    gateStr = "LOCKED";
  } else if (parking.isGateTerbuka()) {
    gateStr = state.mobilSedangDiBawahGate ? "HOLD" : "OPEN";
  }

  String fsmStr = "STANDBY";
  switch (state.status) {
    case STATUS_STANDBY: fsmStr = "STANDBY"; break;
    case STATUS_COUNTDOWN_5S: fsmStr = "COUNTDOWN"; break;
    case STATUS_GATE_TERBUKA: fsmStr = "GATE_OPEN"; break;
    case STATUS_LANE_KELUAR: fsmStr = "EXIT_LANE"; break;
    default: fsmStr = "DEBUG_MODE"; break;
  }

  String json;
  json.reserve(384);
  json = "{\"type\":\"telemetry\"";
  json += ",\"slots\":" + String(state.slotTersedia);
  json += ",\"maxSlots\":" + String(state.kapasitasMaksimal);
  json += ",\"gate\":\"" + gateStr + "\"";
  json += ",\"gateAngle\":" + String(parking.getSudutGate());
  json += ",\"distLeft\":" + String(state.jarakKiri, 1);
  json += ",\"distRight\":" + String(state.jarakKeluar, 1);
  json += ",\"distExit\":" + String(state.jarakKeluar, 1);
  json += ",\"carLeft\":false";
  json += ",\"carRight\":" + String(state.jarakKeluar < AMBANG_DETEKSI_CM ? "true" : "false");
  json += ",\"carExit\":" + String(state.jarakKeluar < AMBANG_DETEKSI_CM ? "true" : "false");
  json += ",\"systemStatus\":\"" + fsmStr + "\"";
  json += ",\"time\":\"" + String(state.bufWaktu) + "\"";
  json += ",\"date\":\"" + String(state.bufTanggal) + "\"";
  json += ",\"day\":\"" + String(state.bufHari) + "\"";
  json += ",\"uptime\":" + String((millis() - waktuMulaiSistem) / 1000);
  json += ",\"ip\":\"" + String(wifi.isTerhubung() ? wifi.getIP() : wifi.getSoftAPIP()) + "\"";
  json += ",\"rssi\":" + String(wifi.getActiveRSSI());
  json += "}";

  wsServer.broadcastTXT(json);
}

void WebDashboardManager::broadcastLog(const char* category, const String &message) {
  String json;
  json.reserve(128);
  json = "{\"type\":\"log\"";
  json += ",\"category\":\"" + String(category) + "\"";
  json += ",\"message\":\"" + message + "\"";
  json += "}";

  wsServer.broadcastTXT(json);
}

void WebDashboardManager::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      IPAddress ip = wsServer.remoteIP(num);
      Serial.printf("[WS] Klien #%u terhubung dari %s\r\n", num, ip.toString().c_str());
      broadcastLog("SYSTEM", "Klien browser baru terhubung.");
      break;
    }

    case WStype_DISCONNECTED: {
      Serial.printf("[WS] Klien #%u terputus.\r\n", num);
      break;
    }

    case WStype_TEXT: {
      String msg = String((char*)payload).substring(0, length);
      handleCommand(msg);
      break;
    }

    default:
      break;
  }
}

int WebDashboardManager::extractJsonInt(const String &json, const String &key, int fallback) {
  String target = "\"" + key + "\"";
  int idx = json.indexOf(target);
  if (idx < 0) return fallback;
  int colonIdx = json.indexOf(":", idx + target.length());
  if (colonIdx < 0) return fallback;
  int start = colonIdx + 1;
  while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) {
    start++;
  }
  int end = start;
  while (end < json.length() && (isDigit(json[end]) || json[end] == '-')) {
    end++;
  }
  if (start == end) return fallback;
  return json.substring(start, end).toInt();
}

String WebDashboardManager::extractJsonString(const String &json, const String &key, const String &fallback) {
  String target = "\"" + key + "\"";
  int idx = json.indexOf(target);
  if (idx < 0) return fallback;
  int colonIdx = json.indexOf(":", idx + target.length());
  if (colonIdx < 0) return fallback;
  int quoteStart = json.indexOf("\"", colonIdx + 1);
  if (quoteStart < 0) return fallback;
  int quoteEnd = json.indexOf("\"", quoteStart + 1);
  if (quoteEnd < 0) return fallback;
  return json.substring(quoteStart + 1, quoteEnd);
}

void WebDashboardManager::handleCommand(const String &jsonStr) {
  // 1. Kontrol Gerbang
  if (jsonStr.indexOf("\"action\":\"gate\"") >= 0) {
    if (jsonStr.indexOf("\"command\":\"OPEN\"") >= 0) {
      parking.bukaManual();
      broadcastLog("GATE", "Portal dibuka manual via remote Web.");
    } else if (jsonStr.indexOf("\"command\":\"CLOSE\"") >= 0) {
      parking.tutupManual();
      broadcastLog("GATE", "Portal ditutup manual via remote Web.");
    } else if (jsonStr.indexOf("\"command\":\"LOCK\"") >= 0) {
      parking.toggleKunciDarurat();
      broadcastLog("GATE", parking.isGateTerkunci() ? "Portal DIKUNCI DARURAT." : "Kunci darurat portal dilepas.");
    }
  }
  // 2. Pengaturan Kuota Slot
  else if (jsonStr.indexOf("\"action\":\"set_slots\"") >= 0) {
    int avail = extractJsonInt(jsonStr, "available", parking.getSlotTersedia());
    int maxVal = extractJsonInt(jsonStr, "max", parking.getKapasitasMaksimal());

    if (maxVal >= 1 && maxVal <= BATAS_MAX_SLOT && avail >= 0 && avail <= maxVal) {
      parking.setKapasitasMaksimal(maxVal);
      parking.setSlotTersedia(avail);
      storage.simpanKapasitasMaksimal(maxVal);
      storage.simpanSlot(avail);
      broadcastLog("CONFIG", "Kuota slot diperbarui: " + String(avail) + " / " + String(maxVal));
    } else {
      broadcastLog("CONFIG", "Gagal: Kuota slot invalid!");
    }
  }
  // 3. Sinkronisasi Waktu RTC dengan Browser
  else if (jsonStr.indexOf("\"action\":\"sync_time\"") >= 0) {
    int h = extractJsonInt(jsonStr, "hour", 12);
    int m = extractJsonInt(jsonStr, "minute", 0);
    int s = extractJsonInt(jsonStr, "second", 0);
    int d = extractJsonInt(jsonStr, "day", 1);
    int mo = extractJsonInt(jsonStr, "month", 1);
    int y = extractJsonInt(jsonStr, "year", 2026);

    struct tm ti = {0};
    ti.tm_year = y - 1900;
    ti.tm_mon  = mo - 1;
    ti.tm_mday = d;
    ti.tm_hour = h;
    ti.tm_min  = m;
    ti.tm_sec  = s;

    struct timeval tv;
    tv.tv_sec = mktime(&ti);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    broadcastLog("CONFIG", "Jam sistem disinkronkan dengan waktu browser: " + String(h) + ":" + String(m) + ":" + String(s));
  }
  // 4. Pengaturan WiFi
  else if (jsonStr.indexOf("\"action\":\"set_wifi\"") >= 0) {
    String ssid = extractJsonString(jsonStr, "ssid");
    String pass = extractJsonString(jsonStr, "pass");

    if (ssid.length() > 0) {
      storage.simpanWiFi(ssid, pass);
      wifi.hubungkan(ssid, pass);
      broadcastLog("CONFIG", "Kredensial WiFi baru disimpan: " + ssid + ". Menghubungkan...");
    }
  }
}
