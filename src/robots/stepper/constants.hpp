#ifndef __STEPPER_CONSTANTS_HPP__

#define __STEPPER_CONSTANTS_HPP__

#include "../debug/debug_inc.h"
#include "../enum.h"
#include "../src/timer/timers/timer_hw.hpp"
#include "../src/device/Memory/EEPROM/AT24CXX/at24c02.hpp"
#include "../src/device/Memory/memory.hpp"

#include "src/device/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "src/device/Encoder/OdometerLines.hpp"
#include "algo/hash_func.hpp"
#include "svpwm.hpp"


static constexpr uint32_t foc_freq = 36000;
static constexpr uint32_t est_freq = foc_freq / 16;
static constexpr uint32_t est_devider = foc_freq / est_freq;

static constexpr int poles = 50;
static constexpr float inv_poles = 1.0 / poles;
static constexpr float openloop_current_limit = 0.3;


static constexpr uint32_t foc_period_micros = 1000000 / foc_freq;

namespace StepperEnums{
    BETTER_ENUM(RunStatus, uint8_t,     
        INIT = 0,
        INACTIVE,
        ACTIVE,
        BEEP,
        CALI,
        ERROR,
        EXCEPTION
    )
};

#endif