#include <cstdint>
#include <cstddef>
#include <new>

#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
  PixelWriter* writer;
  if (frame_buffer_config.pixel_format == kPixelRGBResv8BitPerColor) {
    writer = new (pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
  } else {
    writer = new (pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
  }

  for (int y = 0; y < static_cast<int>(frame_buffer_config.vertical_resolution);
       ++y) {
    for (int x = 0;
         x < static_cast<int>(frame_buffer_config.horizontal_resolution); ++x) {
      writer->Write(x, y, {255, 255, 255});
    }
  }

  WriteAscii(*writer, 8, 8, 'A', {0, 0, 0});

  while (1) __asm__("hlt");
}
