#pragma once

#include <cstdint>

// Keep the small portion of the FreeInk BoardConfig surface used outside the
// device HAL available without pulling ESP32-only GPIO headers into the native
// build. Device and panel-controller selection are compile-time in the
// simulator so each production hardware revision can be exercised explicitly.
#define FREEINK_LOG_TRANSPORT_HWCDC 0
#define FREEINK_LOG_TRANSPORT_ROM_PRINTF 1
#define FREEINK_LOG_TRANSPORT FREEINK_LOG_TRANSPORT_HWCDC

#if (defined(SIMULATOR_DEVICE_X3) + defined(SIMULATOR_DEVICE_X4_PRO) +        \
     defined(SIMULATOR_DEVICE_STICKY) +                                     \
     defined(SIMULATOR_DEVICE_PAPERMONO) +                                  \
     defined(SIMULATOR_DEVICE_EEGO_A4) +                                    \
     (defined(SIMULATOR_DEVICE_MURPHY_M4) ||                                \
      defined(SIMULATOR_DEVICE_MOFEI_M4))) > 1
#error "Select at most one simulated device"
#endif

#if defined(SIMULATOR_DISPLAY_UC8179) && defined(SIMULATOR_DISPLAY_UC8279)
#error "Select at most one simulated display controller"
#endif

#if defined(SIMULATOR_DEVICE_X3) && defined(SIMULATOR_DISPLAY_UC8179)
#error "Xteink X3 revisions use UC8253 or UC8279d, not UC8179"
#endif

#if (defined(SIMULATOR_DEVICE_STICKY) ||                                    \
     defined(SIMULATOR_DEVICE_PAPERMONO) ||                                 \
     defined(SIMULATOR_DEVICE_EEGO_A4) ||                                   \
     defined(SIMULATOR_DEVICE_MURPHY_M4) ||                                 \
     defined(SIMULATOR_DEVICE_MOFEI_M4)) &&                                 \
    (defined(SIMULATOR_DISPLAY_UC8179) || defined(SIMULATOR_DISPLAY_UC8279))
#error "This device has a fixed display controller"
#endif

#undef FREEINK_DEVICE_X4
#undef FREEINK_DEVICE_X3
#undef FREEINK_DEVICE_X4PRO
#undef FREEINK_DEVICE_STICKY
#undef FREEINK_DEVICE_PAPERMONO
#undef FREEINK_DEVICE_EEGO_A4
#undef FREEINK_DEVICE_MURPHY_M4
#undef FREEINK_DEVICE_MOFEI_M4

#if defined(SIMULATOR_DEVICE_PAPERMONO)
#define FREEINK_DEVICE_X4 0
#define FREEINK_DEVICE_X3 0
#define FREEINK_DEVICE_X4PRO 0
#define FREEINK_DEVICE_STICKY 0
#define FREEINK_DEVICE_PAPERMONO 1
#define FREEINK_DEVICE_EEGO_A4 0
#define FREEINK_DEVICE_MURPHY_M4 0
#define FREEINK_DEVICE_MOFEI_M4 0
#define FREEINK_CAP_TOUCH 1
#define FREEINK_CAP_FRONTLIGHT 1
#elif defined(SIMULATOR_DEVICE_STICKY)
#define FREEINK_DEVICE_X4 0
#define FREEINK_DEVICE_X3 0
#define FREEINK_DEVICE_X4PRO 0
#define FREEINK_DEVICE_STICKY 1
#define FREEINK_DEVICE_PAPERMONO 0
#define FREEINK_DEVICE_EEGO_A4 0
#define FREEINK_DEVICE_MURPHY_M4 0
#define FREEINK_DEVICE_MOFEI_M4 0
#define FREEINK_CAP_TOUCH 1
#define FREEINK_CAP_FRONTLIGHT 0
#elif defined(SIMULATOR_DEVICE_EEGO_A4)
#define FREEINK_DEVICE_X4 0
#define FREEINK_DEVICE_X3 0
#define FREEINK_DEVICE_X4PRO 0
#define FREEINK_DEVICE_STICKY 0
#define FREEINK_DEVICE_PAPERMONO 0
#define FREEINK_DEVICE_EEGO_A4 1
#define FREEINK_DEVICE_MURPHY_M4 0
#define FREEINK_DEVICE_MOFEI_M4 0
#define FREEINK_CAP_TOUCH 1
#define FREEINK_CAP_FRONTLIGHT 0
#elif defined(SIMULATOR_DEVICE_MURPHY_M4) ||                                 \
    defined(SIMULATOR_DEVICE_MOFEI_M4)
#define FREEINK_DEVICE_X4 0
#define FREEINK_DEVICE_X3 0
#define FREEINK_DEVICE_X4PRO 0
#define FREEINK_DEVICE_STICKY 0
#define FREEINK_DEVICE_PAPERMONO 0
#define FREEINK_DEVICE_EEGO_A4 0
#define FREEINK_DEVICE_MURPHY_M4 1
#define FREEINK_DEVICE_MOFEI_M4 1
#define FREEINK_CAP_TOUCH 1
#define FREEINK_CAP_FRONTLIGHT 1
#elif defined(SIMULATOR_DEVICE_X4_PRO)
#define FREEINK_DEVICE_X4 0
#define FREEINK_DEVICE_X3 0
#define FREEINK_DEVICE_X4PRO 1
#define FREEINK_DEVICE_STICKY 0
#define FREEINK_DEVICE_PAPERMONO 0
#define FREEINK_DEVICE_EEGO_A4 0
#define FREEINK_DEVICE_MURPHY_M4 0
#define FREEINK_DEVICE_MOFEI_M4 0
#define FREEINK_CAP_TOUCH 1
#define FREEINK_CAP_FRONTLIGHT 1
#elif defined(SIMULATOR_DEVICE_X3)
#define FREEINK_DEVICE_X4 0
#define FREEINK_DEVICE_X3 1
#define FREEINK_DEVICE_X4PRO 0
#define FREEINK_DEVICE_STICKY 0
#define FREEINK_DEVICE_PAPERMONO 0
#define FREEINK_DEVICE_EEGO_A4 0
#define FREEINK_DEVICE_MURPHY_M4 0
#define FREEINK_DEVICE_MOFEI_M4 0
#define FREEINK_CAP_TOUCH 0
#define FREEINK_CAP_FRONTLIGHT 0
#else
#define FREEINK_DEVICE_X4 1
#define FREEINK_DEVICE_X3 0
#define FREEINK_DEVICE_X4PRO 0
#define FREEINK_DEVICE_STICKY 0
#define FREEINK_DEVICE_PAPERMONO 0
#define FREEINK_DEVICE_EEGO_A4 0
#define FREEINK_DEVICE_MURPHY_M4 0
#define FREEINK_DEVICE_MOFEI_M4 0
#define FREEINK_CAP_TOUCH 0
#define FREEINK_CAP_FRONTLIGHT 0
#endif

#define FREEINK_MCU_C3 (FREEINK_DEVICE_X3 || FREEINK_DEVICE_X4)

namespace BoardConfig {

enum class Board {
  XteinkX4,
  XteinkX3,
  XteinkX3Uc8279,
  XteinkX4Pro,
  EegoA4,
  MurphyM4,
  MofeiM4,
  Sticky,
  PaperMono,
};

enum class DisplayController {
  SSD1677,
  UC8253,
  UC8279,
  UC8179,
  UC8279C,
};

struct ViewableInsets {
  uint8_t top = 9;
  uint8_t right = 3;
  uint8_t bottom = 3;
  uint8_t left = 3;
};

struct BoardProfile {
  Board board;
  const char *name;
  DisplayController displayController;
  uint8_t displayControllerVariant;
  struct {
    int8_t up;
    int8_t down;
  } input;
  ViewableInsets viewableInsets = {};
};

#if defined(SIMULATOR_DISPLAY_UC8179)
inline constexpr DisplayController X4_DISPLAY_CONTROLLER =
    DisplayController::UC8179;
inline constexpr uint8_t X4_DISPLAY_CONTROLLER_VARIANT = 0x01;
#elif defined(SIMULATOR_DISPLAY_UC8279)
inline constexpr DisplayController X4_DISPLAY_CONTROLLER =
    DisplayController::UC8279;
// The SDK supports LUT_VER 0x02, 0x68, and reserved 0x69. Model the documented
// 0x68 production variant by default; callers can still inspect the controller.
inline constexpr uint8_t X4_DISPLAY_CONTROLLER_VARIANT = 0x68;
#else
inline constexpr DisplayController X4_DISPLAY_CONTROLLER =
    DisplayController::SSD1677;
inline constexpr uint8_t X4_DISPLAY_CONTROLLER_VARIANT = 0;
#endif

inline constexpr BoardProfile XTEINK_X4 = {Board::XteinkX4, "xteink_x4",
                                           X4_DISPLAY_CONTROLLER,
                                           X4_DISPLAY_CONTROLLER_VARIANT,
                                           {4, 5}};
inline constexpr BoardProfile XTEINK_X3 = {Board::XteinkX3, "xteink_x3",
                                           DisplayController::UC8253, 0, {4, 5}};
inline constexpr BoardProfile XTEINK_X3_UC8279 = {
    Board::XteinkX3Uc8279, "xteink_x3_uc8279", DisplayController::UC8279, 0,
    {4, 5}};
inline constexpr BoardProfile XTEINK_X4_PRO = {
    Board::XteinkX4Pro, "xteink_x4_pro", X4_DISPLAY_CONTROLLER,
    X4_DISPLAY_CONTROLLER_VARIANT, {0, 7}};
inline constexpr BoardProfile EEGO_A4 = {
    Board::EegoA4, "eego_a4", DisplayController::UC8279C, 0, {7, 8}};
inline constexpr BoardProfile MURPHY_M4 = {
    Board::MurphyM4, "murphy_m4", DisplayController::SSD1677, 0, {1, 2}};
inline constexpr BoardProfile MOFEI_M4 = {
    Board::MofeiM4, "mofei_m4", DisplayController::SSD1677, 0, {1, 2}};
inline constexpr BoardProfile STICKY = {
    Board::Sticky, "sticky", DisplayController::SSD1677, 0, {5, 6}};
inline constexpr BoardProfile PAPER_MONO = {
    Board::PaperMono, "m5stack_paper_mono", DisplayController::SSD1677, 0,
    {0, 7}, {9, 7, 3, 7}};

#if defined(SIMULATOR_DEVICE_PAPERMONO)
inline BoardProfile ACTIVE = PAPER_MONO;
#elif defined(SIMULATOR_DEVICE_STICKY)
inline BoardProfile ACTIVE = STICKY;
#elif defined(SIMULATOR_DEVICE_EEGO_A4)
inline BoardProfile ACTIVE = EEGO_A4;
#elif defined(SIMULATOR_DEVICE_MURPHY_M4)
inline BoardProfile ACTIVE = MURPHY_M4;
#elif defined(SIMULATOR_DEVICE_MOFEI_M4)
inline BoardProfile ACTIVE = MOFEI_M4;
#elif defined(SIMULATOR_DEVICE_X4_PRO)
inline BoardProfile ACTIVE = XTEINK_X4_PRO;
#elif defined(SIMULATOR_DEVICE_X3)
#if defined(SIMULATOR_DISPLAY_UC8279)
inline BoardProfile ACTIVE = XTEINK_X3_UC8279;
#else
inline BoardProfile ACTIVE = XTEINK_X3;
#endif
#else
inline BoardProfile ACTIVE = XTEINK_X4;
#endif

inline bool selectDevice(Board board) {
  switch (board) {
  case Board::XteinkX4:
    ACTIVE = XTEINK_X4;
    return true;
  case Board::XteinkX3:
    ACTIVE = XTEINK_X3;
    return true;
  case Board::XteinkX3Uc8279:
    ACTIVE = XTEINK_X3_UC8279;
    return true;
  case Board::XteinkX4Pro:
    ACTIVE = XTEINK_X4_PRO;
    return true;
  case Board::EegoA4:
    ACTIVE = EEGO_A4;
    return true;
  case Board::MurphyM4:
    ACTIVE = MURPHY_M4;
    return true;
  case Board::MofeiM4:
    ACTIVE = MOFEI_M4;
    return true;
  case Board::Sticky:
    ACTIVE = STICKY;
    return true;
  case Board::PaperMono:
    ACTIVE = PAPER_MONO;
    return true;
  }
  return false;
}

inline bool isX4Pro() { return ACTIVE.board == Board::XteinkX4Pro; }
inline bool isEegoA4() { return ACTIVE.board == Board::EegoA4; }
inline bool isMurphyM4() {
  return ACTIVE.board == Board::MurphyM4 ||
         ACTIVE.board == Board::MofeiM4;
}
inline bool isSticky() { return ACTIVE.board == Board::Sticky; }
inline bool isPaperMono() { return ACTIVE.board == Board::PaperMono; }
inline bool hasTouch() {
  return isX4Pro() || isEegoA4() || isMurphyM4() || isSticky() ||
         isPaperMono();
}
inline bool hasHomeKey() { return isX4Pro() || isEegoA4(); }
inline bool hasPwmFrontlight() {
  return isX4Pro() || isMurphyM4() || isPaperMono();
}
inline bool hasColorTemperatureFrontlight() {
  return isX4Pro() || isMurphyM4();
}

inline void holdPowerRails() {}

} // namespace BoardConfig
