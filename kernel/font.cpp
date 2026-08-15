#include "font.hpp"

#include <cstdint>

extern const uint8_t terminus_font[256 * 16];

static void WritePixel(const FrameBufferConfig& config, int x, int y,
                       const PixelColor& color) {
  const int pixel_position = config.pixels_per_scan_line * y + x;
  uint8_t* pixel = &config.frame_buffer[pixel_position * 4];
  if (config.pixel_format == kPixelRGBResv8BitPerColor) {
    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
  } else if (config.pixel_format == kPixelBGRResv8BitPerColor) {
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
  }
}

template <typename Draw>
static void DrawAscii(int x, int y, char c, Draw draw) {
  const uint8_t* font =
      terminus_font + 16 * static_cast<unsigned char>(c);
  for (int dy = 0; dy < 16; ++dy) {
    for (int dx = 0; dx < 8; ++dx) {
      if ((font[dy] << dx) & 0x80u) {
        draw(x + dx, y + dy);
      }
    }
  }
}

void WriteAscii(const FrameBufferConfig& config, int x, int y, char c,
                const PixelColor& color) {
  DrawAscii(x, y, c, [&](int px, int py) { WritePixel(config, px, py, color); });
}

void WriteAscii(PixelWriter& writer, int x, int y, char c,
                const PixelColor& color) {
  DrawAscii(x, y, c, [&](int px, int py) { writer.Write(px, py, color); });
}

void WriteString(const FrameBufferConfig& config, int x, int y, const char* s,
                 const PixelColor& color) {
  for (int i = 0; s[i] != '\0'; i++) {
    WriteAscii(config, x + 8 * i, y, s[i], color);
  }
}

void WriteString(PixelWriter& writer, int x, int y, const char* s,
                 const PixelColor& color) {
  for (int i = 0; s[i] != '\0'; i++) {
    WriteAscii(writer, x + 8 * i, y, s[i], color);
  }
}
