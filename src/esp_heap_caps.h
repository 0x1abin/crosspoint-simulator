#pragma once

#include <cstddef>
#include <cstdlib>

#include "Arduino.h"


#define MALLOC_CAP_EXEC (1 << 0)
#define MALLOC_CAP_32BIT (1 << 1)
#define MALLOC_CAP_8BIT (1 << 2)
#define MALLOC_CAP_DMA (1 << 3)
#define MALLOC_CAP_SPIRAM (1 << 10)
#define MALLOC_CAP_INTERNAL (1 << 11)
#define MALLOC_CAP_DEFAULT (1 << 12)


inline size_t simulatorPsramBytes() { return ESP.getPsramSize(); }

inline size_t heap_caps_get_free_size(const unsigned caps) {
  return (caps & MALLOC_CAP_SPIRAM) ? simulatorPsramBytes() : ESP.getFreeHeap();
}

inline size_t heap_caps_get_largest_free_block(const unsigned caps) {
  return (caps & MALLOC_CAP_SPIRAM) ? simulatorPsramBytes() : ESP.getMaxAllocHeap();
}

inline size_t heap_caps_get_minimum_free_size(const unsigned caps) { return heap_caps_get_free_size(caps); }

inline size_t heap_caps_get_total_size(const unsigned caps) {
  return (caps & MALLOC_CAP_SPIRAM) ? simulatorPsramBytes() : ESP.getHeapSize();
}

inline void* heap_caps_malloc(const size_t size, unsigned /*caps*/) { return std::malloc(size); }

inline void* heap_caps_realloc(void* ptr, const size_t size, unsigned /*caps*/) { return std::realloc(ptr, size); }

inline void heap_caps_free(void* ptr) { std::free(ptr); }
