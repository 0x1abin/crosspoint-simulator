#ifdef NDEBUG
#undef NDEBUG
#endif
#include <BoardConfig.h>
#include <GfxRenderer.h>
#include <HalGPIO.h>
#include <SDL.h>
#include <SimulatorLifecycle.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

GfxRenderer renderer;
ESPMock ESP;

static void testImages() {
  static std::array<uint8_t, HalDisplay::BUFFER_SIZE> source{};
  for (size_t i = 0; i < source.size(); ++i) source[i] = static_cast<uint8_t>((i / 152) ^ i);
  display.drawImage(source.data(), 0, 0, 1216, 684);
  assert(std::memcmp(display.getFrameBuffer(), source.data(), source.size()) == 0);
  display.clearScreen();
  display.drawImageTransparent(source.data(), 0, 0, 1216, 684);
  assert(std::memcmp(display.getFrameBuffer(), source.data(), source.size()) == 0);
  const uint8_t mark = 0x35;
  display.drawImage(&mark, 1208, 683, 8, 1);
  assert(display.getFrameBuffer()[103967] == mark);
  uint32_t loanSize = 0;
  assert(display.lendFrameBufferStorage(&loanSize) && loanSize == 103968);
  assert(!display.beginGrayscale16());
  display.returnFrameBufferStorage();
}

static void pushMouse(SDL_Window* window, SDL_Renderer* render, int x, int y, bool down) {
  SDL_Event event{};
  event.type = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
  event.button.windowID = SDL_GetWindowID(window);
  event.button.button = SDL_BUTTON_LEFT;
  SDL_RenderLogicalToWindow(render, static_cast<float>(x), static_cast<float>(y), &event.button.x, &event.button.y);
  assert(SDL_PushEvent(&event) == 1);
}

static void testTouch() {
  SDL_Window* window = SDL_GetWindowFromID(1);
  assert(window);
  SDL_Renderer* render = SDL_GetRenderer(window);
  assert(render);
  for (bool nativeSize : {false, true}) {
    if (nativeSize) SDL_SetWindowSize(window, renderer.getScreenWidth(), renderer.getScreenHeight());
    SDL_PumpEvents();
    SDL_Delay(20);
    gpio.update();
    for (const auto& point : {std::array<int, 2>{5, 5},
                              {renderer.getScreenWidth() - 6, 5},
                              {5, renderer.getScreenHeight() - 6},
                              {renderer.getScreenWidth() - 6, renderer.getScreenHeight() - 6}}) {
      gpio.beginFrame();
      pushMouse(window, render, point[0], point[1], true);
      gpio.update();
      float nx = 0, ny = 0;
      assert(gpio.wasTouchDown(nx, ny));
      // Check known corners in the native panel frame, independently of scaling.
      int expectedX = point[0], expectedY = point[1];
      switch (renderer.orientation) {
        case GfxRenderer::Portrait:
          expectedX = point[1];
          expectedY = 683 - point[0];
          break;
        case GfxRenderer::PortraitInverted:
          expectedX = 1215 - point[1];
          expectedY = point[0];
          break;
        case GfxRenderer::LandscapeClockwise:
          expectedX = 1215 - point[0];
          expectedY = 683 - point[1];
          break;
        case GfxRenderer::LandscapeCounterClockwise:
          break;
      }
      assert(std::abs(nx * 1215 - expectedX) <= 3);
      assert(std::abs(ny * 683 - expectedY) <= 3);
      pushMouse(window, render, point[0], point[1], false);
      gpio.update();
      assert(gpio.wasTouchTap(nx, ny));
      gpio.beginFrame();
      assert(!gpio.wasTouchTap(nx, ny));
    }
  }
  gpio.beginFrame();
  pushMouse(window, render, 50, 50, true);
  gpio.update();
  SDL_Delay(510);
  gpio.beginFrame();
  gpio.update();
  float nx = 0, ny = 0;
  assert(gpio.wasTouchLongPress(nx, ny));
  gpio.suppressTouchContact();
  pushMouse(window, render, 50, 50, false);
  gpio.update();
  assert(!gpio.wasTouchTap(nx, ny));
}

static uint32_t screenshotPixel(SDL_Surface* image, int x, int y) {
  uint32_t raw = 0;
  std::memcpy(&raw, static_cast<uint8_t*>(image->pixels) + y * image->pitch + x * 4, 4);
  uint8_t r, g, b;
  SDL_GetRGB(raw, image->format, &r, &g, &b);
  assert(r == g && g == b);
  return r;
}

int main(int argc, char** argv) {
  assert(argc == 3);
  if (std::strcmp(argv[1], "wake") == 0) {
    assert(SDL_Init(SDL_INIT_TIMER) == 0);
    SimulatorLifecycle::initProcessArgs(argv);
    gpio.begin();
    if (gpio.getWakeupReason() == HalGPIO::WakeupReason::PowerButton) {
      const auto now = std::chrono::steady_clock::now().time_since_epoch();
      const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() -
                           std::strtoll(std::getenv("READPICO_TEST_SLEEP_START"), nullptr, 10);
      assert(elapsed >= 80);  // UP at 0 ms must not wake before POWER at 80 ms.
      assert(!gpio.wasAnyPressed() && !gpio.wasAnyReleased());
      std::cout << "ReadPico power-only wake/relaunch passed\n";
      return 0;
    }
    const auto start = std::chrono::steady_clock::now().time_since_epoch();
    const auto startMs = std::chrono::duration_cast<std::chrono::milliseconds>(start).count();
    assert(setenv("READPICO_TEST_SLEEP_START", std::to_string(startMs).c_str(), 1) == 0);
    // Only POWER may wake: UP stays down but must not trigger a relaunch.
    assert(setenv("CROSSPOINT_SIM_INPUT_SCRIPT", "0:UP:1000;80:POWER", 1) == 0);
    gpio.startDeepSleep();
    assert(false);
  }
  renderer.orientation = static_cast<GfxRenderer::Orientation>(std::atoi(argv[1]));
  static_assert(HalDisplay::BUFFER_SIZE == 103968);
  assert(BoardConfig::ACTIVE.board == BoardConfig::Board::ReadPico);
  assert(BoardConfig::hasTouch() && !BoardConfig::hasHomeKey());
  assert(!BoardConfig::hasPwmFrontlight());
  assert(display.getGrayscaleLevels() == 16);
  display.begin();
  gpio.begin();
  gpio.beginInput();
  assert(!gpio.isXteinkDevice() && !gpio.hasEdgeSideButtons());
  testImages();
  testTouch();
  display.setInverted(true);
  assert(!display.beginGrayscale16());
  display.setInverted(false);
  assert(display.beginGrayscale16());
  assert(display.toggleInverted());
  assert(!display.commitGrayscale16());
  display.setInverted(false);
  assert(!display.commitGrayscale16());
  uint8_t* native = display.beginGrayscale16();
  assert(native && !display.beginGrayscale16());
  uint32_t size = 42;
  assert(!display.lendFrameBufferStorage(&size) && size == 0);
  native[0] = 0;
  display.cancelGrayscale16();
  assert(!display.commitGrayscale16());
  native = display.beginGrayscale16();
  assert(native && native[0] == 255);
  for (size_t i = 0; i < 1216 * 684 / 2; ++i)
    native[i] = static_cast<uint8_t>(((2 * i + 1) % 16) << 4 | ((2 * i) % 16));
  assert(display.commitGrayscale16() && !display.commitGrayscale16());
  const std::string schedule = std::string("0:") + argv[2] + ";100:" + argv[2] + ".cancel.bmp";
  assert(setenv("CROSSPOINT_SIM_SCREENSHOTS", schedule.c_str(), 1) == 0);
  display.presentIfNeeded();
  SDL_Surface* image = SDL_LoadBMP(argv[2]);
  assert(image && image->w == renderer.getScreenWidth() && image->h == renderer.getScreenHeight());
  // Native origin is at a different corner in each oriented screenshot.
  const int originX =
      renderer.orientation == GfxRenderer::Portrait || renderer.orientation == GfxRenderer::LandscapeClockwise
          ? image->w - 1
          : 0;
  const int originY =
      renderer.orientation == GfxRenderer::PortraitInverted || renderer.orientation == GfxRenderer::LandscapeClockwise
          ? image->h - 1
          : 0;
  for (int i = 0; i < 16; ++i) {
    int x = originX, y = originY;
    switch (renderer.orientation) {
      case GfxRenderer::Portrait:
        y += i;
        break;
      case GfxRenderer::PortraitInverted:
        y -= i;
        break;
      case GfxRenderer::LandscapeClockwise:
        x -= i;
        break;
      case GfxRenderer::LandscapeCounterClockwise:
        x += i;
        break;
    }
    assert(screenshotPixel(image, x, y) == static_cast<unsigned>(i * 17));
  }
  native = display.beginGrayscale16();
  assert(native);
  std::memset(native, 0, HalDisplay::BUFFER_SIZE * 4);
  display.cancelGrayscale16();
  SDL_Delay(120);
  display.presentIfNeeded();
  const std::string cancelPath = std::string(argv[2]) + ".cancel.bmp";
  SDL_Surface* cancelImage = SDL_LoadBMP(cancelPath.c_str());
  assert(cancelImage && cancelImage->w == image->w && cancelImage->h == image->h);
  assert(cancelImage->pitch == image->pitch);
  assert(std::memcmp(image->pixels, cancelImage->pixels, image->pitch * image->h) == 0);
  SDL_FreeSurface(cancelImage);
  SDL_FreeSurface(image);
  SDL_Quit();
  std::cout << "ReadPico display/input checks passed: " << argv[1] << '\n';
}
