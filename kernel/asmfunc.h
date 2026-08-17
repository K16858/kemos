#pragma once

#include <stdint.h>

extern "C" {
void LoadIDT(uint16_t limit, void* offset);
void IntHandlerInt40(void);
}
