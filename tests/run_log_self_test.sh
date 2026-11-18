#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/sim-log.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT
"${CXX:-c++}" -std=c++20 -pthread -I"$repo_root/src" \
  "$repo_root/tests/sim-log.cpp" "$repo_root/src/SimLog.cpp" -o "$test_dir/check"
for mode in lines bytes partial threads; do "$test_dir/check" "$mode"; done
printf 'log line, byte, partial and concurrency limits passed\n'
