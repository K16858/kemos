#pragma once

#include "graphics.hpp"
#include "frame_buffer_config.hpp"

void WriteAscii(const FrameBufferConfig& config, int x, int y, char c,
                const PixelColor& color);
