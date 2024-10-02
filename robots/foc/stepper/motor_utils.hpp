#ifndef __STEPPER_CONSTANTS_HPP__

#define __STEPPER_CONSTANTS_HPP__

#include "sys/debug/debug_inc.h"
#include "algo/hash_func.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "thirdparty/enums/enum.h"

#include "drivers/Actuator/SVPWM/svpwm.hpp"

#include "drivers/Memory/EEPROM/AT24CXX/at24cxx.hpp"
#include "drivers/Memory/memory.hpp"

#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Encoder/odometer_poles.hpp"

// scexpr uint32_t foc_freq = 32768;
// scexpr uint32_t chopper_freq = 32768;
// scexpr uint32_t foc_freq = 8192 * 4;
scexpr uint32_t foc_freq = 20000;
// scexpr uint32_t foc_freq = 38000;
// scexpr uint32_t foc_freq = 18000;
// scexpr uint32_t foc_freq = 15000;
// scexpr uint32_t foc_freq = 12000;
scexpr uint32_t chopper_freq = 32768;


scexpr real_t pi_4 = real_t(PI/4);
scexpr real_t pi_2 = real_t(PI/2);
scexpr real_t pi = real_t(PI);
scexpr real_t tau = real_t(TAU);

scexpr int poles = 50;
scexpr real_t inv_poles = real_t(1) / poles;

scexpr uint32_t foc_period_micros = 1000000 / foc_freq;


struct MetaData{
    real_t accel = 0;
    real_t curr = 0;
    real_t spd = 0;
    real_t pos = 0;
    real_t elecrad = 0;
    real_t raddiff = 0;
    real_t radfix = 0;

    real_t openloop_curr;

    Range pos_limit = Range::INF;
    real_t max_curr = real_t(0.7);
    int max_spd = 30;
    int max_acc = 30;
    
    real_t curr_to_leadrad_ratio = real_t(1);
    real_t spd_to_leadrad_ratio = real_t(0.1);
    real_t max_leadrad = real_t(0.2);
    
    real_t targ_curr;
    
    real_t targ_est_spd;
    real_t targ_pos;
    
    real_t targ_spd;
    
    void reset();
    
    __fast_inline constexpr real_t get_max_leadrad();
    __fast_inline constexpr real_t get_max_raddiff();
};

constexpr real_t MetaData::get_max_leadrad(){
    return MIN( ABS(curr) * curr_to_leadrad_ratio, 
                ABS(spd) * spd_to_leadrad_ratio,
                max_leadrad);
}

constexpr real_t MetaData::get_max_raddiff(){
    return get_max_leadrad() + pi_2;
}


namespace MotorUtils{

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
        SPEED,
        TEACH
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
        SET_TRG_DELTA,       //delta target position and switch to position-closeloop mode
        SET_TRG_SPD,//          set target speed and switch to speed-closeloop mode
        SET_TRG_TEACH,//        set max current and switch to teach mode
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
        GET_ALL = 0x3f,         // get all

        TRIG_CALI = 0X40,//          trig cali

        SAVE = 0X50,//          save archive
        LOAD,//                 load archive
        CLEAR,//                clear archive

        RST = 0x70,//           chip reset
        STAT,//                 get stat
        INACTIVE,//             switch to inactive
        ACTIVE,//               switch to active
        SET_NODEID,//           set node id
        SET_NODEID_BY_CRC,//    set node id by chip crc
    };

    using ExitFlag = bool;
    using InitFlag = bool;

    template<integral T>
    struct NodeId_t{
        T id_;

        NodeId_t(const T _id):id_(_id){}
        operator T() const{return id_;}
    };

    using NodeId =  NodeId_t<uint8_t>;
}

#endif