# EEGO A4

This profile follows CrossPoint's `eego_a4` environment added in
[firmware PR 3887](https://github.com/crosspoint-reader/crosspoint-reader/pull/3887)
and initially used SDK `425d200a8ea447326b4b9696e4e47dc84ad9d7f6`, pinned by
CrossPoint develop `43694c9018d26446ab30cce2fa65ee608355ae50`.

The current CrossMux rehearsal reviews SDK
`7fcd3b34c108f28c086ade3b433924022dbc0998`, Reader
`ba39565ce7b3a8aab14ca86da4fa4c3ca140f007` and Simulator
`1915ee0a15564c3f49d74fa2334f773f282c5364`, preserving the fork's clock and
local device compatibility. Integration validation is recorded separately from
the historical upstream application-build attempt below.

The source of truth is the SDK's
[board profile](https://github.com/Free-Ink/freeink-sdk/blob/7fcd3b34c108f28c086ade3b433924022dbc0998/libs/hardware/BoardConfig/include/BoardConfig.h)
and [hardware notes](https://github.com/Free-Ink/freeink-sdk/blob/7fcd3b34c108f28c086ade3b433924022dbc0998/docs/eego-a4-support.md).
The SDK documents incomplete physical hardware validation. This host profile
does not validate those provisional pin assignments or controller waveforms.

## Profile Contract

- Native framebuffer: 768x552, UC8279C, 52,992 bytes at one bit per pixel.
  Portrait UI dimensions are 552x768; landscape dimensions are 768x552.
- Viewable insets: top/right/bottom/left = 24/24/8/24, transformed by the renderer.
- GSLX680 touch coordinates use the existing orientation-aware host tap/swipe path.
- Physical Up/Down/Power only: keyboard Up/Down/P. Escape, Return and Left/Right
  are disabled, rather than adding navigation buttons the device lacks.
- H emits the GSLX680 screen-key press, short-release and 700 ms hold events.
  Page Up/Down do not emit Metalio cover events.
- PCF8563-compatible clock availability, backed by host time; no tilt sensor or haptics.
- Optional LM3630A frontlight state: brightness and warm/cool balance. The host
  models state only, not I2C, illumination or electrical power sequencing.

`simulator_eego_a4` models a frontlit unit. `simulator_eego_a4_no_frontlight`
models a missing frontlight chip: `present()` and color-temperature availability
are false, and the light cannot turn on. Both retain `FREEINK_CAP_FRONTLIGHT=1`,
as the same hardware firmware binary probes for the optional controller.
The board reports an I2C frontlight, not a PWM frontlight.

The SDK's nominal UI-scale metadata is 1.2. Current CrossPoint uses one fixed
UI-font tier in `src/components/UIScale.h`; this profile does not force a
different scale or change the firmware UI policy.

## Home-Key Contract

The reviewed SDK's `EEGO_A4.touch.hasHomeKey` is true in
[BoardConfig.h](https://github.com/Free-Ink/freeink-sdk/blob/7fcd3b34c108f28c086ade3b433924022dbc0998/libs/hardware/BoardConfig/include/BoardConfig.h).
The simulator reports that capability and delivers H press, short-release and
hold events. CrossPoint's mapped-input layer can therefore handle them through
the same capability gate as the device. Home does not synthesize Back edges.
The A4 display reports overlay masks, a separate base, and no strip or async
uploads, matching the reviewed UC8279C SDK driver. The native tests cover the events;
full application builds and screenshots require separate validation.

## Verification

From the simulator repository:

```sh
python3 tests/check-profile-selection.py
sh tests/run-eego.sh
sh tests/run-metalio.sh
```

The profile test checks configured boards and rejects mixed devices,
incompatible controllers and a lightless flag without EEGO. Native SDL tests
exercise both light variants, the framebuffer size and margins, button
availability, screen-key events, clock/tilt capabilities, frontlight state,
edge-coordinate taps and swipes in all four orientations. Metalio is rechecked
because it shares the input and frontlight paths.

Use either sample PlatformIO configuration to add the two environments to a
firmware checkout. Simulator evidence is not physical e-paper, touch firmware,
RTC, frontlight-probe or ESP32 memory validation.

### Historical upstream application-build attempt

The `develop-lpla` application build was attempted at `792e4abbb`.
That port added the missing `BatteryMonitor::loadDesignCapacity()` host shim
needed by recent upstream firmware; it returns false because no hardware
gauge work is pending on the host.

The full application did not build at that revision. Its compatibility blockers were:
`TrustedTime` calls undeclared `settimeofday`, the protected-book headers
redefine `ContentDecryptor`, and the integrated GIF branch requires
`StaticSemaphore_t`, which the host FreeRTOS shim does not provide yet.
A local, uncommitted clock fixture that refuses host clock changes exposed
the latter two errors. No application screenshot or end-to-end reading test
was claimed for that attempt. The native SDL input tests above run without that
fixture. Those results do not determine the current rehearsal's build status.
