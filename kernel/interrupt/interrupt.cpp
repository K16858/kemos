#include "interrupt/interrupt.hpp"

#include "interrupt/asmfunc.h"

namespace {

union InterruptDescriptorAttribute {
  uint16_t data;
  struct {
    uint16_t interrupt_stack_table : 3;
    uint16_t : 5;
    uint16_t type : 4;
    uint16_t : 1;
    uint16_t descriptor_privilege_level : 2;
    uint16_t present : 1;
  } __attribute__((packed)) bits;
} __attribute__((packed));

struct InterruptDescriptor {
  uint16_t offset_low;
  uint16_t segment_selector;
  InterruptDescriptorAttribute attr;
  uint16_t offset_middle;
  uint32_t offset_high;
  uint32_t reserved;
} __attribute__((packed));

alignas(16) InterruptDescriptor idt[256];

Console* g_console = nullptr;

void SetIDTEntry(InterruptDescriptor& desc, InterruptDescriptorAttribute attr,
                 uint64_t offset, uint16_t segment_selector) {
  desc.attr = attr;
  desc.offset_low = static_cast<uint16_t>(offset & 0xffffu);
  desc.offset_middle = static_cast<uint16_t>((offset >> 16) & 0xffffu);
  desc.offset_high = static_cast<uint32_t>(offset >> 32);
  desc.segment_selector = segment_selector;
  desc.reserved = 0;
}

InterruptDescriptorAttribute MakeAttr(uint16_t type, uint16_t dpl) {
  InterruptDescriptorAttribute attr{};
  attr.bits.interrupt_stack_table = 0;
  attr.bits.type = type;
  attr.bits.descriptor_privilege_level = dpl;
  attr.bits.present = 1;
  return attr;
}

uint16_t GetCS() {
  uint16_t cs;
  __asm__("mov %%cs, %0" : "=r"(cs));
  return cs;
}

}  // namespace

extern "C" void IntHandlerInt40_C() {
  if (g_console != nullptr) {
    g_console->PutString("int 0x40\n");
  }
}

void SetupInterrupt(Console* console) {
  g_console = console;

  for (int i = 0; i < 256; ++i) {
    idt[i].offset_low = 0;
    idt[i].segment_selector = 0;
    idt[i].attr.data = 0;
    idt[i].offset_middle = 0;
    idt[i].offset_high = 0;
    idt[i].reserved = 0;
  }

  SetIDTEntry(idt[kInterruptVectorNumber], MakeAttr(14, 0),
              reinterpret_cast<uint64_t>(IntHandlerInt40), GetCS());
  SetIDTEntry(idt[kInterruptVectorIRQ1], MakeAttr(14, 0),
              reinterpret_cast<uint64_t>(IntHandlerKeyboard), GetCS());
  LoadIDT(sizeof(idt) - 1, reinterpret_cast<void*>(idt));
}
