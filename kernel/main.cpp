#include <cstdint>
#include <cstddef>
#include <new>

#include "console.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "interrupt.hpp"

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  __asm__("cli");

  char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
  PixelWriter* writer;
  if (frame_buffer_config.pixel_format == kPixelRGBResv8BitPerColor) {
    writer = new (pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
  } else {
    writer = new (pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
  }

  const PixelColor black{0, 0, 0};
  const PixelColor white{255, 255, 255};
  for (int y = 0;
       y < static_cast<int>(frame_buffer_config.vertical_resolution); ++y) {
    for (int x = 0;
         x < static_cast<int>(frame_buffer_config.horizontal_resolution); ++x) {
      writer->Write(x, y, black);
    }
  }

  Console console{*writer, white, black};
  console.PutString("Hello, KEMOS!\n");

  SetupInterrupt(&console);
  __asm__("int $0x40");

  while (1) __asm__("hlt");
}
