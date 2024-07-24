#pragma once

#include "../../sys/core/platform.h"

uint16_t fast_square8(const uint8_t x);
#define FAST_SQUARE8(x) (fast_square8(x))