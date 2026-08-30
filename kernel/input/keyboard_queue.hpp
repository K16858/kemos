#pragma once

#include <cstdint>

void KeyboardQueuePush(uint8_t scancode);
bool KeyboardQueuePop(uint8_t* scancode);
