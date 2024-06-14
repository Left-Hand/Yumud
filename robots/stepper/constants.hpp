#ifndef __STEPPER_CONSTANTS_HPP__

#define __STEPPER_CONSTANTS_HPP__

#include "../debug/debug_inc.h"
#include "../thirdparty/enums/enum.h"
#include "../hal/timer/timers/timer_hw.hpp"
#include "../drivers/Memory/EEPROM/AT24CXX/at24c02.hpp"
#include "../drivers/Memory/memory.hpp"

#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Encoder/OdometerLines.hpp"
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
        NONE,
        INIT,
        INACTIVE,
        ACTIVE,
        CHECK,
        BEEP,
        CALI,
        ERROR,
        EXIT,
        WARN
    )

    BETTER_ENUM(CtrlType, uint8_t,     
        POSITION = 0,
        SPEED
    )

    enum class ErrorCode:uint8_t{
        OK = 0,
        COIL_A_DISCONNECTED,
        COIL_A_NO_SIGNAL,
        COIL_B_DISCONNECTED,
        COIL_B_NO_SIGNAL,
        ODO_DISCONNECTED,
        ODO_NO_SIGNAL
    };
};

#endif