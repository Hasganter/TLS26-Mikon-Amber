#ifndef KEYPAD_MANAGER_H
#define KEYPAD_MANAGER_H

#include <Arduino.h>
#include <Keypad.h>
#include "../config/Config.h"

class KeypadManager {
public:
  KeypadManager();

  // Dapatkan tombol yang ditekan (mengembalikan NO_KEY jika tidak ada)
  char bacaTombol();

private:
  char petaTombol[KEYPAD_BARIS][KEYPAD_KOLOM];
  byte pinBaris[KEYPAD_BARIS];
  byte pinKolom[KEYPAD_KOLOM];
  Keypad keypad;
};

#endif // KEYPAD_MANAGER_H
