#include "KeypadInputHelper.h"

const char* KeypadInputHelper::MAP_ABC[] = {
  " 0",        // 0
  ".,?!@-_/1", // 1
  "ABC2",      // 2
  "DEF3",      // 3
  "GHI4",      // 4
  "JKL5",      // 5
  "MNO6",      // 6
  "PQRS7",     // 7
  "TUV8",      // 8
  "WXYZ9"      // 9
};

const char* KeypadInputHelper::MAP_abc[] = {
  " 0",        // 0
  ".,?!@-_/1", // 1
  "abc2",      // 2
  "def3",      // 3
  "ghi4",      // 4
  "jkl5",      // 5
  "mno6",      // 6
  "pqrs7",     // 7
  "tuv8",      // 8
  "wxyz9"      // 9
};

const char* KeypadInputHelper::MAP_123[] = {
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

KeypadInputHelper::KeypadInputHelper()
  : buffer(""),
    currentMode(MODE_abc),
    lastKey('\0'),
    cycleIndex(0),
    lastKeyTime(0),
    pendingChar('\0') {}

void KeypadInputHelper::reset(const String &initialText) {
  buffer = initialText;
  lastKey = '\0';
  cycleIndex = 0;
  lastKeyTime = 0;
  pendingChar = '\0';
}

void KeypadInputHelper::setMode(InputMode mode) {
  commitPending();
  currentMode = mode;
}

void KeypadInputHelper::toggleMode() {
  commitPending();
  if (currentMode == MODE_ABC) {
    currentMode = MODE_abc;
  } else if (currentMode == MODE_abc) {
    currentMode = MODE_123;
  } else {
    currentMode = MODE_ABC;
  }
}

const char* KeypadInputHelper::getCharListForKey(char key) const {
  if (key < '0' || key > '9') return nullptr;
  int idx = key - '0';
  if (currentMode == MODE_ABC) return MAP_ABC[idx];
  if (currentMode == MODE_abc) return MAP_abc[idx];
  return MAP_123[idx];
}

void KeypadInputHelper::commitPending() {
  if (pendingChar != '\0') {
    buffer += pendingChar;
    pendingChar = '\0';
    lastKey = '\0';
    cycleIndex = 0;
  }
}

void KeypadInputHelper::backspace() {
  if (pendingChar != '\0') {
    pendingChar = '\0';
    lastKey = '\0';
    cycleIndex = 0;
  } else if (buffer.length() > 0) {
    buffer.remove(buffer.length() - 1);
  }
}

bool KeypadInputHelper::handleKey(char key) {
  unsigned long now = millis();

  // Tombol D: Pengalih Mode
  if (key == 'D') {
    toggleMode();
    return true;
  }

  // Tombol *: Backspace
  if (key == '*') {
    backspace();
    return true;
  }

  // Tombol B: Quick Space
  if (key == 'B') {
    commitPending();
    buffer += ' ';
    return true;
  }

  // Tombol A: Commit langsung / Next
  if (key == 'A') {
    commitPending();
    return true;
  }

  // Tombol Numerik 0-9
  if (key >= '0' && key <= '9') {
    // Jika mode 123: langsung masukkan angka
    if (currentMode == MODE_123) {
      commitPending();
      buffer += key;
      return true;
    }

    const char* chars = getCharListForKey(key);
    if (!chars) return false;
    int numChars = strlen(chars);

    // Jika tombol sama ditekan dalam batas waktu multi-tap
    if (key == lastKey && (now - lastKeyTime < MULTI_TAP_TIMEOUT_MS) && pendingChar != '\0') {
      cycleIndex = (cycleIndex + 1) % numChars;
      pendingChar = chars[cycleIndex];
      lastKeyTime = now;
    } else {
      // Commit karakter sebelumnya jika ada
      commitPending();
      lastKey = key;
      cycleIndex = 0;
      pendingChar = chars[0];
      lastKeyTime = now;
    }
    return true;
  }

  return false;
}

void KeypadInputHelper::update() {
  if (pendingChar != '\0') {
    if (millis() - lastKeyTime >= MULTI_TAP_TIMEOUT_MS) {
      commitPending();
    }
  }
}

String KeypadInputHelper::getCommittedText() const {
  String result = buffer;
  if (pendingChar != '\0') {
    result += pendingChar;
  }
  return result;
}

String KeypadInputHelper::getDisplayText(bool showCursor) const {
  String out = buffer;
  if (pendingChar != '\0') {
    out += pendingChar;
  }
  if (showCursor) {
    bool blink = (millis() / 500) % 2;
    out += (blink ? "_" : " ");
  }
  return out;
}

InputMode KeypadInputHelper::getMode() const {
  return currentMode;
}

const char* KeypadInputHelper::getModeStr() const {
  switch (currentMode) {
    case MODE_ABC: return "ABC";
    case MODE_abc: return "abc";
    case MODE_123: return "123";
  }
  return "abc";
}

bool KeypadInputHelper::isPending() const {
  return (pendingChar != '\0');
}

int KeypadInputHelper::length() const {
  return buffer.length() + (pendingChar != '\0' ? 1 : 0);
}
