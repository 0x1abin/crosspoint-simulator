#pragma once

#include <cstdint>
#include <random>

inline uint32_t esp_random() { return std::random_device{}(); }
