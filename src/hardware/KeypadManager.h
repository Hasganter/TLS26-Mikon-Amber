#pragma once
#include <Arduino.h>
#include <Keypad.h>
#include "../config/Config.h"

class KeypadManager {
public:
  KeypadManager();

  // Dapatkan tombol yang ditekan (mengembalikan NO_KEY jika tidak ada)
  char bacaTombol();

  // Status kesehatan keypad
  bool isOk() const;

private:
  char petaTombol[KEYPAD_BARIS][KEYPAD_KOLOM];
  byte pinBaris[KEYPAD_BARIS];
  byte pinKolom[KEYPAD_KOLOM];
  Keypad keypad;
};

