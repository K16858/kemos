#include <cstdint>
#include <cstddef>

#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

int WritePixel(const FrameBufferConfig& config, int x, int y,
               const PixelColor& color) {
  const int pixel_position = config.pixels_per_scan_line * y + x;
  if (config.pixel_format == kPixelRGBResv8BitPerColor) {
    uint8_t* pixel = &config.frame_buffer[pixel_position * 4];
    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
  } else if (config.pixel_format == kPixelBGRResv8BitPerColor) {
    uint8_t* pixel = &config.frame_buffer[pixel_position * 4];
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
  } else {
    return -1;
  }
  return 0;
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  for (int y = 0;
       y < static_cast<int>(frame_buffer_config.vertical_resolution); ++y) {
    for (int x = 0;
         x < static_cast<int>(frame_buffer_config.horizontal_resolution); ++x) {
      WritePixel(frame_buffer_config, x, y, {255, 255, 255});
    }
  }

  for (int x = 0; x < 200; ++x) {
    for (int y = 0; y < 100; ++y) {
      WritePixel(frame_buffer_config, 100 + x, 100 + y, {0, 255, 0});
    }
  }

  WriteAscii(frame_buffer_config, 8, 8, 'A', {0, 0, 0});

  while (1) __asm__("hlt");
}
