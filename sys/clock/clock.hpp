#pragma once

#include "clock.h"

#include "sys/math/iq/iq_t.hpp"
#include <functional>
namespace Sys{
extern iq_t t;
};
void bindSystickCb(std::function<void(void)> && cb);