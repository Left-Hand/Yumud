#pragma once

#include "drivers/GateDriver/DRV832X/DRV8323h.hpp"
#include "drivers/Encoder/MagEnc/MT6825/mt6825.hpp"
#include "drivers/Encoder/MagEnc/VCE2755/vce2755.hpp"
#include "motor_leso.hpp"

//常见电机参数：
// https://item.taobao.com/item.htm?id=643573104607



namespace ymd::myesc{

//磁结构
//直观理解表贴于内置式的磁路
// https://blog.csdn.net/u010632165/article/details/103637894
enum class [[nodiscard]] MagneticStructure{
    //表贴式永磁同步电机(Surface-Mounted Permanent Magnet Synchronous Motor)
    SurfaceMounted,
    //内嵌式永磁同步电机(Interior Permanent Magnet Synchronous Motor)
    Interior
};

// static constexpr uint32_t CHOPPER_FREQ = 32_KHz;
static constexpr uint32_t CHOPPER_FREQ = 25_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

static constexpr auto BUS_VOLT = iq16(12.0);
static constexpr auto INV_BUS_VOLT = 1 / BUS_VOLT;
static constexpr size_t HFI_FREQ = 1000;

using Leso = ymd::dsp::adrc::MotorLeso;

struct MotorProfile_Gim6010{
    //伺泰威关节电机
    static constexpr size_t POLE_PAIRS = 10u;
    // static constexpr auto PHASE_INDUCTANCE = 0.0085_iq20;
    // static constexpr auto PHASE_INDUCTANCE = 0.00245_iq20;
    // static constexpr auto PHASE_INDUCTANCE = 0.0025_iq20;

    //100uh
    static constexpr auto PHASE_INDUCTANCE = iq20(22.3 * 1E-6);

    //1ohm
    // static constexpr auto PHASE_RESISTANCE = 1.123_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.123_iq20;
};

struct MotorProfile_Ysc{
    //云深处关节电机
    static constexpr size_t POLE_PAIRS = 7u;
    static constexpr auto PHASE_INDUCTANCE = iq20(180 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.303_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.145_uq16);

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto MODU_VOLT_LIMIT = iq16(5.5);
    static constexpr auto LESO_B0 = 30;

    static constexpr iq16 MACHINE_KP = 1.73_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.075_iq16;

    using MagEncoder = drivers::MT6825;
};

struct MotorProfile_3505{
    //3505航模电机
    //具有良好的凸极性
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 7u;
    static constexpr auto PHASE_INDUCTANCE = iq20(20 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.203_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.145_uq16);

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto MODU_VOLT_LIMIT = iq16(5.5);
    static constexpr auto LESO_B0 = 30;

    static constexpr iq16 MACHINE_KP = 1.73_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.075_iq16;

    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_Gim4010{
    //伺泰威关节电机
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(300 * 1E-6);
    static constexpr auto PHASE_RESISTANCE = 1.03_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(7.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 1600;
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;
    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_36BLDB{
    //苏州凯航电机
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(200 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 2.57_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(7.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto leso_coeffs = Leso::Config{
        .fs = FOC_FREQ,
        // .fc = 2000,
        .fc = 50,
        // .b0 = 1000
        .b0 = 30
    }.try_into_coeffs().unwrap();
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;
    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_NiuLiu{
    //筋膜枪电机
    //具有良好的扭矩和凸极性 价格美丽

    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(200 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 2.57_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(7.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 400;
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;
    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_M06Bare{
    //本末M06剪线电机（又名ddsm400)
    //!不具有任何凸极性
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::SurfaceMounted;
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(2200 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    // static constexpr auto PHASE_RESISTANCE = 3.03_iq20;
    static constexpr auto PHASE_RESISTANCE = 2.45_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(7.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 700;
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;
    using MagEncoder = drivers::VCE2755;
};


}