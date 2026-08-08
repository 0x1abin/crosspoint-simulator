#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
test_binary="$(mktemp "${TMPDIR:-/tmp}/crosspoint-simulator-compat.XXXXXX")"
trap 'rm -f "$test_binary"' EXIT

compile=(
  -std=gnu++20
  -Wno-deprecated-declarations
  "-I$repo_root/src"
  "$repo_root/tests/host_compat_self_test.cpp"
  "$repo_root/src/HalClock.cpp"
  "$repo_root/src/HalFrontlight.cpp"
  "$repo_root/src/HalStorage.cpp"
  "$repo_root/src/HalSystem.cpp"
)
if [[ -n "${LDFLAGS:-}" ]]; then
  read -r -a extra_link_flags <<< "$LDFLAGS"
  compile+=("${extra_link_flags[@]}")
fi
if [[ "$(uname -s)" == "Linux" ]]; then
  compile+=(-lcrypto)
fi
for device in DEFAULT SIMULATOR_DEVICE_X3 SIMULATOR_DEVICE_X4_PRO \
  SIMULATOR_DEVICE_EEGO_A4 SIMULATOR_DEVICE_MOFEI_M4; do
  device_compile=("${CXX:-c++}" "${compile[@]}")
  if [[ "$device" != DEFAULT ]]; then
    device_compile+=("-D$device")
  fi
  device_compile+=(-o "$test_binary")
  "${device_compile[@]}"
  "$test_binary"
done
printf 'host compatibility self-test passed\n'
