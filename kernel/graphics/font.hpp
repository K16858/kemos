#pragma once

#include "graphics/graphics.hpp"
#include "graphics/frame_buffer_config.hpp"

void WriteAscii(const FrameBufferConfig& config, int x, int y, char c,
                const PixelColor& color);
void WriteAscii(PixelWriter& writer, int x, int y, char c,
                const PixelColor& color);

void WriteString(const FrameBufferConfig& config, int x, int y, const char* s,
                 const PixelColor& color);
void WriteString(PixelWriter& writer, int x, int y, const char* s,
                 const PixelColor& color);
