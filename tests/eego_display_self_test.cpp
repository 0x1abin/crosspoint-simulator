#include <BoardConfig.h>
#include <GfxRenderer.h>
#include <HalDisplay.h>

#include <cassert>

GfxRenderer renderer;
ESPMock ESP;

int main() {
  assert(BoardConfig::isEegoA4());
  const auto overlay = display.grayscaleCapabilities();
  assert(overlay.encoding == HalDisplay::GrayscaleEncoding::OverlayMasks);
  assert(overlay.base == HalDisplay::GrayscaleBase::Separate);
  assert(!overlay.stripUploads && !overlay.asyncBase && !overlay.stagingWhileBusy);
  assert(!display.grayscaleCapabilities(HalDisplay::GrayscaleMode::Absolute).supported());
  assert(!display.grayscaleCapabilities(HalDisplay::GrayscaleMode::Direct).supported());
}
