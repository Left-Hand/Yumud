#pragma once

#include <cstdint>
#include "hal/bus/can/can.hpp"
#include "core/math/realmath.hpp"

// https://github.com/enactic/openarm_can/tree/main/include/openarm/damiao_motor

namespace ymd::robots::damiao{

enum class MotorKind : uint8_t {
    DM3507 = 0,
    DM4310 = 1,
    DM4310_48V = 2,
    DM4340 = 3,
    DM4340_48V = 4,
    DM6006 = 5,
    DM8006 = 6,
    DM8009 = 7,
    DM10010L = 8,
    DM10010 = 9,
    DMH3510 = 10,
    DMH6215 = 11,
    DMG6220 = 12,
};

enum class RID : uint8_t {
    UV_Value = 0,
    KT_Value = 1,
    OT_Value = 2,
    OC_Value = 3,
    ACC = 4,
    DEC = 5,
    MAX_SPD = 6,
    MST_ID = 7,
    ESC_ID = 8,
    TIMEOUT = 9,
    CTRL_MODE = 10,
    Damp = 11,
    Inertia = 12,
    hw_ver = 13,
    sw_ver = 14,
    SN = 15,
    NPP = 16,
    Rs = 17,
    LS = 18,
    Flux = 19,
    Gr = 20,
    PMAX = 21,
    VMAX = 22,
    TMAX = 23,
    I_BW = 24,
    KP_ASR = 25,
    KI_ASR = 26,
    KP_APR = 27,
    KI_APR = 28,
    OV_Value = 29,
    GREF = 30,
    Deta = 31,
    V_BW = 32,
    IQ_c1 = 33,
    VL_c1 = 34,
    can_br = 35,
    sub_ver = 36,
    u_off = 50,
    v_off = 51,
    k1 = 52,
    k2 = 53,
    m_off = 54,
    dir = 55,
    p_m = 80,
    xout = 81,
    COUNT = 82
};

// Limit parameters structure for different motor types
struct LimitParam {
    real_t max_position;  // Position limit (rad)
    real_t max_speed;  // Velocity limit (rad/s)
    real_t max_torque;  // Torque limit (Nm)
};


namespace details{
template<MotorKind K>
struct LimitParamTable;

#define DEF_MOTOR_LIMIT_PARAM(K, pMax, vMax, tMax) \
template<> \
struct LimitParamTable<K> { \
    static constexpr LimitParam table = {real_t(pMax), real_t(vMax), real_t(tMax)}; \
};

DEF_MOTOR_LIMIT_PARAM(MotorKind::DM3507, 12.5, 50, 5)
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM4310, 12.5, 30, 10) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM4310_48V, 12.5, 50, 10) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM4340, 12.5, 8, 28)  
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM4340_48V, 12.5, 10, 28) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM6006, 12.5, 45, 20) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM8006, 12.5, 45, 40) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM8009, 12.5, 45, 54) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM10010L, 12.5, 25, 200)
DEF_MOTOR_LIMIT_PARAM(MotorKind::DM10010, 12.5, 20, 200)
DEF_MOTOR_LIMIT_PARAM(MotorKind::DMH3510, 12.5, 280, 1) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DMH6215, 12.5, 45, 10) 
DEF_MOTOR_LIMIT_PARAM(MotorKind::DMG6220, 12.5, 45, 10) 

#undef DEF_MOTOR_LIMIT_PARAM
}


namespace msgs{
struct ParamResult {
    int rid;
    real_t value;
    bool valid;
};

struct StateResult {
    real_t position;
    real_t speed;
    real_t torque;
    int t_mos;
    int t_rotor;
    bool valid;
};

struct MITParam {
    real_t kp;
    real_t kd;
    real_t q;
    real_t dq;
    real_t tau;
};
}
}