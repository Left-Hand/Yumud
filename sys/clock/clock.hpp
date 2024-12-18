#pragma once

#include "clock.h"

#include "sys/math/iq/iq_t.hpp"
#include <functional>

void bindSystickCb(std::function<void(void)> && cb);