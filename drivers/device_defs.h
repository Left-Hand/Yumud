#pragma once

#include <bit>
#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <optional>

#include "sys/io/regs.hpp"
#include "sys/math/fraction.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/bus/uart/uart.hpp"

#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/adc/analog_channel.hpp"

// using namespace ymd::hal;