#pragma once

namespace ymd::foc{
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
        COIL_C_DISCONNECTED,
        COIL_C_NO_SIGNAL,
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
        SET_ACC_LMT,//          set global accration limit

        GET_POS = 0x30,//       get position
        GET_SPD,//              get speed
        read_acc,//              get accration
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

}