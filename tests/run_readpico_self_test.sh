#!/usr/bin/env bash
set -euo pipefail
sim_root="$(cd "$(dirname "$0")/.." && pwd)"
project_root="${1:?Pass the CrossMux project root}"
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/readpico-display.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT
"${CXX:-c++}" -std=c++20 -Wall -Wextra -Wno-unused-parameter \
  -DSIMULATOR -DCROSSPOINT_EMULATED=1 -DSIMULATOR_DEVICE_READPICO \
  -I"$sim_root/tests/display_stubs" -I"$sim_root/src" -I"$project_root/lib/Memory" \
  "$sim_root/tests/readpico_display_self_test.cpp" "$sim_root/src/HalDisplay.cpp" \
  "$sim_root/src/HalGPIO.cpp" "$sim_root/src/SimulatorLifecycle.cpp" \
  $(sdl2-config --cflags --libs) -pthread -o "$test_dir/check"
for orientation in 0 1 2 3; do
  "$test_dir/check" "$orientation" "$test_dir/$orientation.bmp"
done
"$test_dir/check" wake ignored
