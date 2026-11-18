#pragma once
#include <HalDisplay.h>
#include <cmath>

// Real host HAL with firmware geometry/scale; no production rendering substitute.
class GfxRenderer {
 public:
  enum Orientation { Portrait, LandscapeClockwise, PortraitInverted, LandscapeCounterClockwise };
  Orientation orientation = Portrait;
  Orientation getOrientation() const { return orientation; }
  int getScreenWidth() const {
    const bool portrait = orientation == Portrait || orientation == PortraitInverted;
    return static_cast<int>(std::lround((portrait ? HalDisplay::DISPLAY_HEIGHT : HalDisplay::DISPLAY_WIDTH) /
                                       BoardConfig::ACTIVE.uiScale));
  }
  int getScreenHeight() const {
    const bool portrait = orientation == Portrait || orientation == PortraitInverted;
    return static_cast<int>(std::lround((portrait ? HalDisplay::DISPLAY_WIDTH : HalDisplay::DISPLAY_HEIGHT) /
                                       BoardConfig::ACTIVE.uiScale));
  }
};
