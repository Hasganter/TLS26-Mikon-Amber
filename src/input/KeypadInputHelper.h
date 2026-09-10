#pragma once
#include <Arduino.h>
#include "../config/SystemTypes.h"
#include "../config/Config.h"

class KeypadInputHelper {
public:
  KeypadInputHelper();

  // Reset buffer teks dan status input
  void reset(const String &initialText = "");

  // Atur mode input langsung
  void setMode(InputMode mode);

  // Beralih mode: ABC -> abc -> 123 -> ABC (tombol D)
  void toggleMode();

  // Proses penekanan tombol, return true jika ditangani
  bool handleKey(char key);

  // Periksa timeout 800ms untuk commit otomatis karakter pending
  void update();

  // Commit karakter pending seketika
  void commitPending();

  // Hapus karakter (backspace)
  void backspace();

  // Dapatkan teks yang sudah ter-commit
  String getCommittedText() const;

  // Dapatkan teks untuk tampilan layar (disertai karakter pending & kursor)
  String getDisplayText(bool showCursor = true) const;

  // Dapatkan mode saat ini
  InputMode getMode() const;
  const char* getModeStr() const;

  // Status apakah ada karakter yang sedang berputar (pending)
  bool isPending() const;

  // Panjang total teks saat ini
  int length() const;

private:
  String buffer;
  InputMode currentMode;
  char lastKey;
  int cycleIndex;
  unsigned long lastKeyTime;
  char pendingChar;

  static const char* MAP_ABC[];
  static const char* MAP_abc[];
  static const char* MAP_123[];

  const char* getCharListForKey(char key) const;
};

