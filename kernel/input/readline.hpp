#pragma once

#include "graphics/console.hpp"

#include <cstddef>

void InitializeReadLine(Console* console);
void ReadLinePushChar(char c);
bool ReadLineTryGetLine(char* out, size_t max_len);
