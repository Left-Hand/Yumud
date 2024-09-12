#pragma once
// #include <sstl/queue.h>
// #include <sstl/deque.h>

#include "types/real.hpp"

#include "dsp/filter/LowpassFilter.hpp"

#include "hal/gpio/port_virtual.hpp"
// #include "hal/debug/debug_inc.h"
#include "hal/exti/exti.hpp"
#include "hal/timer/timer.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/timer/capture/capture_channel.hpp"
#include "hal/timer/timer_oc.hpp"

#include "hal/adc/adcs/adc1.hpp"

#include "drivers/Adc/INA226/ina226.hpp"
#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "drivers/Adc/HX711/HX711.hpp"
#include "drivers/Wireless/Radio/HC12/HC12.hpp"

#include "dsp/controller/PID.hpp"

#include "types/image/painter.hpp"

void wlsy_main();