#pragma once
#include <Arduino.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"
#include "../system/StorageManager.h"
#include "../system/TimeManager.h"
#include "../network/WiFiManager.h"
#include "../input/KeypadInputHelper.h"
#include "ParkingController.h"

class DebugController {
public:
  DebugController(
    ParkingController &parking,
    StorageManager &storage,
    WiFiManager &wifi
  );

  void handleLoop(StatusSistem &status, char tombol, unsigned long sekarang);
  void populateUIState(UIState &state);

private:
  ParkingController &parking;
  StorageManager &storage;
  WiFiManager &wifi;
  KeypadInputHelper inputHelper;

  unsigned long waktuAktivitasTerakhir;
  unsigned long waktuFeedback;
  String pesanFeedback;

  // Buffer input standar
  String bufferInput;

  // Buffer slot
  bool fokusMaksimal;
  String bufferTersedia;
  String bufferMaksimal;

  // WiFi scan selection
  int wifiSelectedIndex;
  int wifiScrollOffset;
  String wifiTargetSSID;

  void resetActivity(unsigned long sekarang);
};

