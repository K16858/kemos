#include "keyboard.hpp"

#include "asmfunc.h"
#include "pic.hpp"

namespace {

constexpr uint16_t kKeyboardDataPort = 0x0060;

Console* g_keyboard_console = nullptr;

const char keycode_map[0x80] = {
    0,    0,    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',
    '=',  '\b', 0,    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',
    '[',  ']',  '\n', 0,    'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',
    ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',
    '.',  '/',  0,    '*',  0,    ' ',
};

char KeycodeToChar(uint8_t keycode) {
  if ((keycode & 0x80) != 0) {
    return 0;
  }
  if (keycode >= sizeof(keycode_map)) {
    return 0;
  }
  return keycode_map[keycode];
}

}  // namespace

extern "C" void IntHandlerKeyboard_C() {
  const uint8_t keycode = IoIn8(kKeyboardDataPort);
  const char c = KeycodeToChar(keycode);
  if (c != 0 && g_keyboard_console != nullptr) {
    char buf[2] = {c, '\0'};
    g_keyboard_console->PutString(buf);
  }
  NotifyEndOfInterrupt();
}

void InitializeKeyboard(Console* console) {
  g_keyboard_console = console;
  EnableKeyboardInterrupt();
}
