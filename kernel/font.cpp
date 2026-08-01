#include "font.hpp"

#include <cstdint>

extern const uint8_t terminus_font[256 * 16];

void WriteAscii(PixelWriter& writer, int x, int y, char c,
                const PixelColor& color) {
  const uint8_t* font =
      terminus_font + 16 * static_cast<unsigned char>(c);
  for (int dy = 0; dy < 16; ++dy) {
    for (int dx = 0; dx < 8; ++dx) {
      if ((font[dy] << dx) & 0x80u) {
        writer.Write(x + dx, y + dy, color);
      }
    }
  }
}
