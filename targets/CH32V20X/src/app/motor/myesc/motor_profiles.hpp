#pragma once

#include "motor_prelude.hpp"

//常见电机参数：
// https://item.taobao.com/item.htm?id=643573104607

namespace ymd::myesc{


struct MotorProfile_Gim6010{
    //伺泰威关节电机
    static constexpr size_t POLE_PAIRS = 10u;

    //100uh
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(22.3 * 1E-3);

    //1ohm
    static constexpr auto PHASE_RESISTANCE_OHM = 0.123_iq20;
};

struct MotorProfile_Ysc{
    //云深处关节电机
    static constexpr size_t POLE_PAIRS = 7u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(180 * 1E-3);
    // static constexpr auto PHASE_INDUCTANCE_MH = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE_OHM = 0.303_iq20;

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto LESO_B0 = 30;


};

struct MotorProfile_3505{
    //3505航模电机
    //具有良好的凸极性
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 10u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(42 * 1E-3);
    static constexpr auto FLUX_LINKAGE = iq20(8.4 * 1E-4);
    // static constexpr auto PHASE_INDUCTANCE_MH = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE_OHM = 0.103_iq20;

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 800;

};


struct MotorProfile_E800{
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 10u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(76 * 1E-3);
    static constexpr auto FLUX_LINKAGE = iq20(8.4 * 1E-4);
    static constexpr auto PHASE_RESISTANCE_OHM = 0.227_iq20;

    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 800;

};

// https://item.taobao.com/item.htm?id=744363121525
struct MotorProfile_NidecFan{
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 10u;

    static constexpr auto Q_AXIS_INDUCTANCE = iq20(180 * 1E-3);
    static constexpr auto D_AXIS_INDUCTANCE = iq20(137 * 1E-3);
    static constexpr auto QD_RATIO = 180.0 / 137;

    static constexpr auto PHASE_INDUCTANCE_MH = (Q_AXIS_INDUCTANCE + D_AXIS_INDUCTANCE) >> 1;
    static constexpr auto FLUX_LINKAGE = iq20(8.4 * 1E-4);
    static constexpr auto PHASE_RESISTANCE_OHM = 0.07_iq20;

    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 800;

};

struct MotorProfile_2207{
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 7u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(13 * 1E-3);
    static constexpr auto PHASE_RESISTANCE_OHM = 0.112_iq20;
    static constexpr auto FLUX_LINKAGE = iq20(3.4 * 1E-4);
    static constexpr auto CURRENT_CUTOFF_FREQ = 500;

};


struct MotorProfile_Gim4010{
    //伺泰威关节电机
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(300 * 1E-3);
    static constexpr auto PHASE_RESISTANCE_OHM = 1.03_iq20;
    static constexpr auto CURRENT_CUTOFF_FREQ = 1600;
};

struct MotorProfile_36BLDB{
    //苏州凯航电机
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 4u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(200 * 1E-3);
    static constexpr auto PHASE_RESISTANCE_OHM = 2.57_iq20;
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-3);
    static constexpr auto CURRENT_CUTOFF_FREQ = 600;
};


struct MotorProfile_NiuLiu{
    //筋膜枪电机
    //具有良好的扭矩和凸极性 价格美丽

    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(200 * 1E-3);
    static constexpr auto PHASE_RESISTANCE_OHM = 2.57_iq20;
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-3);
    static constexpr auto CURRENT_CUTOFF_FREQ = 500;


};


struct MotorProfile_M06Bare{
    //本末M06剪线电机（又名ddsm400)
    //!不具有任何凸极性
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::SurfaceMounted;
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(2200 * 1E-3);
    static constexpr auto PHASE_RESISTANCE_OHM = 2.45_iq20;
    static constexpr auto CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-3);
};

struct MotorProfile_Wheel{
    //!不具有任何凸极性
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::SurfaceMounted;
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE_MH = iq20(86.24 * 1E-3);
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-3);
    static constexpr auto PHASE_RESISTANCE_OHM = 0.0645_iq20;
    static constexpr auto CURRENT_CUTOFF_FREQ = 300;
};


}