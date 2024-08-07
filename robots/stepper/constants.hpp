#ifndef __STEPPER_CONSTANTS_HPP__

#define __STEPPER_CONSTANTS_HPP__

#include "../sys/debug/debug_inc.h"
#include "../thirdparty/enums/enum.h"
#include "../hal/timer/instance/timer_hw.hpp"
#include "../drivers/Memory/EEPROM/AT24CXX/at24cxx.hpp"
#include "../drivers/Memory/memory.hpp"

#include "../drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "../drivers/Encoder/OdometerLines.hpp"
#include "../algo/hash_func.hpp"
#include "svpwm.hpp"


static constexpr uint32_t foc_freq = 36000;
static constexpr uint32_t chopper_freq = 36000;
static constexpr uint32_t est_freq = foc_freq / 16;
static constexpr uint32_t est_devider = foc_freq / est_freq;

static constexpr int poles = 50;
static constexpr float inv_poles = 1.0 / poles;
static constexpr float openloop_current_limit = 0.3;


static constexpr uint32_t foc_period_micros = 1000000 / foc_freq;


// #define STEPPER_NO_PRINT

#ifdef STEPPER_NO_PRINT
#define CLI_PRINTS(...)
#define ARCHIVE_PRINTS(...)
#define CLI_DEBUG(...)
#define COMMAND_DEBUG(...)
#define RUN_DEBUG(...)

#else
#define CLI_PRINTS(...) logger.prints(__VA_ARGS__);
#define ARCHIVE_PRINTS(...) if(outen) logger.prints(__VA_ARGS__);

#define CALI_DEBUG(...)\
if(cali_debug_enabled){\
logger.println(__VA_ARGS__);};

#define COMMAND_DEBUG(...)\
if(command_debug_enabled){\
logger.println(__VA_ARGS__);};

#define RUN_DEBUG(...)\
if(run_debug_enabled){\
logger.println(__VA_ARGS__);};

#endif

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
        SET_TARGET = 0,//       set target, do not change control mode
        SET_TRG_VECT,//         set target position and switch to position-openloop mode
        SET_TRG_CURR,//         set target current and switch to current-closeloop mode
        SET_TRG_POS,//          set target position and switch to position-closeloop mode
        SET_TRG_SPD,//          set target speed and switch to speed-closeloop mode
        SET_TRG_TPZ,//          set target position and switch to position-trapzoid mode
        FREEZE = 0x0F,//        freeze stepper, lock position

        CFG_CURR_CTL = 0x10,//  config current controller
        CFG_POS_CTL,//          config position controller
        CFG_SPD_CTL,//          config speed controller
        CFG_TPZ_CTL,//          config trapzoid controller
        CFG_STALL_DTT,//        config stall detector
        CFG_JERK_DTT,//         config jerk detector

        LOCATE = 0x20,
        SET_OPEN_CURR,
        SET_CURR_LMT,//         set global current limit
        SET_POS_LMT,//          set global position limit [a, b]
        SET_SPD_LMT,//          set global speed limit
        SET_ACC_LMT,//          set global accelration limit

        GET_POS = 0x30,//       get position
        GET_SPD,//              get speed
        GET_ACC,//              get accelration
        GET_CURR,//             get current

        TRG_CALI = 0X40,//          trig cali

        SAVE = 0X50,//          save archive
        LOAD,//                 load archive
        CLEAR,//                clear archive

        NOZZLE_ON = 0x60,//     turn on nozzle
        NOZZLE_OFF,//           turn off nozzle

        RST = 0x70,//           chip reset
        STAT,//                 get stat
        INACTIVE,//             switch to inactive
        ACTIVE,//               switch to active
        SET_NODEID,//           set node id
        SET_NODEID_BY_CRC,//    set node id by chip crc
    };

    using ExitFlag = bool;
    using InitFlag = bool;
};

#endif