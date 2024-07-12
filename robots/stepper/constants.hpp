#ifndef __STEPPER_CONSTANTS_HPP__

#define __STEPPER_CONSTANTS_HPP__

#include "sys/debug/debug_inc.h"
#include "../thirdparty/enums/enum.h"
#include "../hal/timer/instance/timer_hw.hpp"
#include "../drivers/Memory/EEPROM/AT24CXX/at24cxx.hpp"
#include "../drivers/Memory/memory.hpp"

#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Encoder/OdometerLines.hpp"
#include "algo/hash_func.hpp"
#include "svpwm.hpp"


static constexpr uint32_t foc_freq = 36000;
static constexpr uint32_t chopper_freq = 36000;
static constexpr uint32_t est_freq = foc_freq / 16;
static constexpr uint32_t est_devider = foc_freq / est_freq;

static constexpr int poles = 50;
static constexpr float inv_poles = 1.0 / poles;
static constexpr float openloop_current_limit = 0.3;


static constexpr uint32_t foc_period_micros = 1000000 / foc_freq;

namespace StepperEnums{

    enum class RunStatus:uint8_t{
        NONE,
        INIT,
        INACTIVE,
        ACTIVE,
        IDLE,
        CHECK,
        BEEP,
        CALI,
        ERROR,
        WARN,
        EXIT
    };

    // BETTER_ENUM(CtrlType, uint8_t, 
    enum class CtrlType:uint8_t{
        VECTOR,
        CURRENT,
        POSITION,
        TRAPEZOID,
        SPEED
    };
    
    enum class ErrorCode:uint8_t{
        OK = 0,
        ODO_DISCONNECTED,
        ODO_NO_SIGNAL,
        COIL_A_DISCONNECTED,
        COIL_A_NO_SIGNAL,
        COIL_B_DISCONNECTED,
        COIL_B_NO_SIGNAL,
    };

    enum class Command:uint8_t{
        SET_TARGET = 0,
        TRG_VECT,
        TRG_CURR,
        TRG_POS,
        TRG_SPD,
        TRG_TPZ,

        LOCATE = 0x20,
        SET_OLP_CURR,
        CLAMP_CURRENT,
        CLAMP_POS,
        CLAMP_SPD,
        CLAMP_ACC,

        GET_POS = 0x30,
        GET_SPD,
        GET_ACC,
        GET_CURR,

        CALI = 0X40,

        SAVE = 0X50,
        LOAD,
        RM,

        SERVO_ON = 0x60,
        SERVO_OFF,

        RST = 0x70,
        STAT,
        INACTIVE,
        ACTIVE,
        SET_NODEID,
    };

    using ExitFlag = bool;
    using InitFlag = bool;
};

#endif