#pragma once

#include "../../core/platform.hpp"

uint16_t fast_square8(const uint8_t x);
#define FAST_SQUARE8(x) (fast_square8(x))