extern "C" void KernelMain(uint64_t framebuffer_base, uint64_t framebuffer_size) {
    uint8_t* frame_buffer = reinterpret_cast<uint8_t*>(framebuffer_base);
    for (uint64_t i = 0; i < framebuffer_size; i++) {
        frame_buffer[i] = i % 256;
    }
    while (1) __asm__("hlt");
}
