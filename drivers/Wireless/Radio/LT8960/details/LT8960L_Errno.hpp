#pragma once

#include "../LT8960L.hpp"
#include "core/debug/debug.hpp"
#include "core/container/ringbuf.hpp"

#include "hal/gpio/gpio.hpp"


#define LT8960L_DEBUG_EN

#ifdef LT8960L_DEBUG_EN
#define LT8960L_TODO(...) TODO()
#define LT8960L_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define LT8960L_PANIC(...) PANIC{__VA_ARGS__}
#define LT8960L_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define LT8960L_DEBUG(...)
#define LT8960L_TODO(...) PANIC_NSRC()
#define LT8960L_PANIC(...)  PANIC_NSRC()
#define LT8960L_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

namespace ymd::drivers{

using Error = drivers::LT8960L::Error;

static constexpr size_t LT8960L_MAX_PACKET_SIZE = 12;
static constexpr size_t LT8960L_MAX_FIFO_SIZE = 16;

}