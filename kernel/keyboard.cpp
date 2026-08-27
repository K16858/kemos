#include "keyboard.hpp"

#include "asmfunc.h"
#include "pic.hpp"

namespace {

constexpr uint16_t kKeyboardDataPort = 0x0060;

Console* g_keyboard_console = nullptr;

void PutHex8(Console& console, uint8_t value) {
  static const char kHex[] = "0123456789abcdef";
  char buf[4];
  buf[0] = kHex[(value >> 4) & 0xf];
  buf[1] = kHex[value & 0xf];
  buf[2] = ' ';
  buf[3] = '\0';
  console.PutString(buf);
}

}  // namespace

extern "C" void IntHandlerKeyboard_C() {
  const uint8_t keycode = IoIn8(kKeyboardDataPort);
  if (g_keyboard_console != nullptr) {
    PutHex8(*g_keyboard_console, keycode);
  }
  NotifyEndOfInterrupt();
}

void InitializeKeyboard(Console* console) {
  g_keyboard_console = console;
  EnableKeyboardInterrupt();
}
