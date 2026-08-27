#include "keyboard.hpp"

#include "asmfunc.h"
#include "pic.hpp"

namespace {

constexpr uint16_t kKeyboardDataPort = 0x0060;
constexpr uint8_t kKeyLeftShift = 0x2a;
constexpr uint8_t kKeyRightShift = 0x36;

Console* g_keyboard_console = nullptr;
bool shift_pressed = false;

const char keycode_map[0x80] = {
    0,    0,    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',
    '=',  '\b', 0,    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',
    '[',  ']',  '\n', 0,    'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',
    ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',
    '.',  '/',  0,    '*',  0,    ' ',
};

const char keycode_map_shifted[0x80] = {
    0,    0,    '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',
    '+',  '\b', 0,    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',
    '{',  '}',  '\n', 0,    'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',
    ':',  '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '<',
    '>',  '?',  0,    '*',  0,    ' ',
};

char KeycodeToChar(uint8_t keycode) {
  if (keycode >= 0x80) {
    return 0;
  }
  const char* map = shift_pressed ? keycode_map_shifted : keycode_map;
  return map[keycode];
}

bool UpdateShiftState(uint8_t keycode) {
  if (keycode == kKeyLeftShift || keycode == kKeyRightShift) {
    shift_pressed = true;
    return true;
  }
  if (keycode == static_cast<uint8_t>(kKeyLeftShift | 0x80) ||
      keycode == static_cast<uint8_t>(kKeyRightShift | 0x80)) {
    shift_pressed = false;
    return true;
  }
  return false;
}

}  // namespace

extern "C" void IntHandlerKeyboard_C() {
  const uint8_t keycode = IoIn8(kKeyboardDataPort);
  if (!UpdateShiftState(keycode)) {
    const char c = KeycodeToChar(keycode);
    if (c != 0 && g_keyboard_console != nullptr) {
      char buf[2] = {c, '\0'};
      g_keyboard_console->PutString(buf);
    }
  }
  NotifyEndOfInterrupt();
}

void InitializeKeyboard(Console* console) {
  g_keyboard_console = console;
  shift_pressed = false;
  EnableKeyboardInterrupt();
}
