#include "input/keyboard_queue.hpp"

#include <cstddef>

namespace {

constexpr size_t kQueueSize = 256;

uint8_t buffer[kQueueSize];
volatile size_t head = 0;
volatile size_t tail = 0;

}  // namespace

void KeyboardQueuePush(uint8_t scancode) {
  const size_t next = (head + 1) % kQueueSize;
  if (next == tail) {
    return;
  }
  buffer[head] = scancode;
  head = next;
}

bool KeyboardQueuePop(uint8_t* scancode) {
  if (tail == head || scancode == nullptr) {
    return false;
  }
  *scancode = buffer[tail];
  tail = (tail + 1) % kQueueSize;
  return true;
}
