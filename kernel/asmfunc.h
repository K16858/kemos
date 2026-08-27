#pragma once

#include <stdint.h>

extern "C" {
void LoadIDT(uint16_t limit, void* offset);
void IntHandlerInt40(void);
void IoOut8(uint16_t addr, uint8_t data);
uint8_t IoIn8(uint16_t addr);
}
