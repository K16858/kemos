#pragma once

#include <cstdint>

#include "console.hpp"

const int kInterruptVectorNumber = 0x40;

void SetupInterrupt(Console* console);
