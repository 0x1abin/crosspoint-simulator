#pragma once
#include <HalDisplay.h>

// Display/input tests use the real host HAL; the firmware owns orientation.
class GfxRenderer {
 public:
  enum Orientation { Portrait, LandscapeClockwise, PortraitInverted, LandscapeCounterClockwise };
  Orientation orientation = Portrait;
  Orientation getOrientation() const { return orientation; }
  int getScreenWidth() const {
    return orientation == Portrait || orientation == PortraitInverted ? HalDisplay::DISPLAY_HEIGHT
                                                                      : HalDisplay::DISPLAY_WIDTH;
  }
  int getScreenHeight() const {
    return orientation == Portrait || orientation == PortraitInverted ? HalDisplay::DISPLAY_WIDTH
                                                                      : HalDisplay::DISPLAY_HEIGHT;
  }
};
