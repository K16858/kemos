#include "pic.hpp"

#include "asmfunc.h"

namespace {

constexpr uint16_t kPic1Command = 0x0020;
constexpr uint16_t kPic1Data = 0x0021;
constexpr uint16_t kPic2Command = 0x00a0;
constexpr uint16_t kPic2Data = 0x00a1;

}  // namespace

void InitializePIC() {
  IoOut8(kPic1Data, 0xff);
  IoOut8(kPic2Data, 0xff);

  IoOut8(kPic1Command, 0x11);
  IoOut8(kPic2Command, 0x11);

  IoOut8(kPic1Data, 0x20);
  IoOut8(kPic2Data, 0x28);

  IoOut8(kPic1Data, 1u << 2);
  IoOut8(kPic2Data, 2);

  IoOut8(kPic1Data, 0x01);
  IoOut8(kPic2Data, 0x01);

  IoOut8(kPic1Data, 0xff);
  IoOut8(kPic2Data, 0xff);
}

void NotifyEndOfInterrupt() {
  IoOut8(kPic1Command, 0x20);
}

void EnableKeyboardInterrupt() {
  IoOut8(kPic1Data, 0xfd);
  IoOut8(kPic2Data, 0xff);
}
