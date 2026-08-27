#pragma once

#include <cstdint>

#include "console.hpp"

const int kInterruptVectorNumber = 0x40;
const int kInterruptVectorIRQ1 = 0x21;

void SetupInterrupt(Console* console);
