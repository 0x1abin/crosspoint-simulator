#include <BoardConfig.h>
#include <GfxRenderer.h>
#include <HalDisplay.h>

#include <cassert>
#include <array>

GfxRenderer renderer;
ESPMock ESP;

#include "../src/HalDisplay.cpp"

int main() {
  assert(BoardConfig::isEegoA4());
  const auto overlay = display.grayscaleCapabilities();
  assert(overlay.encoding == HalDisplay::GrayscaleEncoding::OverlayMasks);
  assert(overlay.base == HalDisplay::GrayscaleBase::Separate);
  assert(!overlay.stripUploads && !overlay.asyncBase && !overlay.stagingWhileBusy);
  static std::array<uint8_t, HalDisplay::BUFFER_SIZE> lsb{}, msb{};
  lsb[0] = 0b01010000;
  msb[0] = 0b00110000;
  for (auto mode : {HalDisplay::GrayscaleMode::Absolute, HalDisplay::GrayscaleMode::Direct}) {
    const auto caps = display.grayscaleCapabilities(mode);
    assert(caps.encoding == HalDisplay::GrayscaleEncoding::AbsolutePlanes);
    assert(caps.base == HalDisplay::GrayscaleBase::Combined);
    assert(!caps.stripUploads && !caps.asyncBase && !caps.stagingWhileBusy);
    assert(display.displayGrayscaleBase(mode));
    assert(!display.displayCommitted());
    display.copyGrayscaleBuffers(lsb.data(), msb.data());
    display.displayGrayBuffer();
    assert(display.displayCommitted());
    assert((pixelBuf[0] & 255) == 0 && (pixelBuf[1] & 255) == 96);
    assert((pixelBuf[2] & 255) == 200 && (pixelBuf[3] & 255) == 255);
  }
  display.clearScreen();
  display.getFrameBuffer()[0] = 0b10001111;
  lsb[0] = 0b00100000;
  msb[0] = 0b00110000;
  assert(display.displayGrayscaleBase(HalDisplay::GrayscaleMode::Overlay));
  display.copyGrayscaleBuffers(lsb.data(), msb.data());
  display.displayGrayBuffer();
  assert((pixelBuf[0] & 255) == 255 && (pixelBuf[1] & 255) == 0);
  assert((pixelBuf[2] & 255) == 96 && (pixelBuf[3] & 255) == 200);
  // Masks on a white B/W base must still become gray, matching the SDK's fold.
  display.clearScreen();
  assert(display.displayGrayscaleBase(HalDisplay::GrayscaleMode::Overlay));
  display.copyGrayscaleBuffers(lsb.data(), msb.data());
  display.displayGrayBuffer();
  assert((pixelBuf[2] & 255) == 96 && (pixelBuf[3] & 255) == 200);
  assert(display.displayGrayscaleBase(HalDisplay::GrayscaleMode::Direct));
  display.cancelGrayscale();
  assert(!display.displayCommitted());
  std::puts("EEGO overlay/absolute/direct capabilities, exact composed pixels and cancellation passed");
}
