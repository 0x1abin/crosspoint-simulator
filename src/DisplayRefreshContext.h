#pragma once
#if __has_include(<../hal/DisplayRefreshContext.h>)
// GfxRenderer consumers can expose its directory without exposing lib/hal.
#include <../hal/DisplayRefreshContext.h>
#elif __has_include_next(<DisplayRefreshContext.h>)
// Reuse a directly exposed HAL type; standalone fixtures use the fallback.
#include_next <DisplayRefreshContext.h>
#else
#include <stdint.h>

// HAL-owned request intent; no persistent permission crosses display calls.
enum class DisplayRefreshContext : uint8_t { Normal, ContinuousReading, TextOnlyAntiAliasing, ImageReading };

#endif
