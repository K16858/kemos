#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

struct PixelColor {
    uint8_t r, g, b;
}

int WritePixel(const FrameBufferConfig& config, int x, int y, const PixelColor& color) {
    const int pixel_position = config.pixels_per_scan_line * y + x;
    if (config.pixel_format == kPixelRGBResv8BitPerColor) {
        uint8_t* pixel = &config.frame_buffer[pixel_position * 4];
        pixel[0] = color.r;
        pixel[1] = color.g;
        pixel[2] = color.b;
    }
    else if (config.pixel_format == kPixelBGRResv8BitPerColor) {
        uint8_t* pixel = &config.frame_buffer[pixel_position * 4];
        pixel[0] = color.b;
        pixel[1] = color.g;
        pixel[2] = color.r;
    }
    else {
        return -1;
    }
    return 0;
}

extern "C" void KernelMain(uint64_t framebuffer_base, uint64_t framebuffer_size) {
    uint8_t* frame_buffer = reinterpret_cast<uint8_t*>(framebuffer_base);
    for (uint64_t i = 0; i < framebuffer_size; i++) {
            frame_buffer[i] = i % 256;
    }
    while (1) __asm__("hlt");
}
