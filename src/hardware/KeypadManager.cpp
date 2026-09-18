#include "KeypadManager.h"

KeypadManager::KeypadManager()
  : petaTombol{
      { '1', '2', '3', 'A' },
      { '4', '5', '6', 'B' },
      { '7', '8', '9', 'C' },
      { '*', '0', '#', 'D' }
    },
    pinBaris{ PIN_KEYPAD_BARIS[0], PIN_KEYPAD_BARIS[1], PIN_KEYPAD_BARIS[2], PIN_KEYPAD_BARIS[3] },
    pinKolom{ PIN_KEYPAD_KOLOM[0], PIN_KEYPAD_KOLOM[1], PIN_KEYPAD_KOLOM[2], PIN_KEYPAD_KOLOM[3] },
    keypad(makeKeymap(petaTombol), pinBaris, pinKolom, KEYPAD_BARIS, KEYPAD_KOLOM) {}

char KeypadManager::bacaTombol() {
  return keypad.getKey();
}

bool KeypadManager::isOk() const {
  return true;
}
