#pragma once

#include "drivers/encoder/magnetic/MT6825/mt6825.hpp"
#include "drivers/encoder/magnetic/VCE2755/vce2755.hpp"
#include "motor_leso.hpp"

//常见电机参数：
// https://item.taobao.com/item.htm?id=643573104607

namespace ymd::myesc{

using namespace ymd::digipw;

using namespace ymd::dsp;

//磁结构
//直观理解表贴于内置式的磁路
// https://blog.csdn.net/u010632165/article/details/103637894
enum class [[nodiscard]] MagneticStructure{
    //表贴式永磁同步电机(Surface-Mounted Permanent Magnet Synchronous Motor)
    SurfaceMounted,
    //内嵌式永磁同步电机(Interior Permanent Magnet Synchronous Motor)
    Interior
};


enum class [[nodiscard]] ElecAngleSource:uint8_t{
    Openloop = 0,
    Hall,
    Hfi,
    MagEncoder,
    AbzEncoder
};

enum class [[nodiscard]] SenlessObserverMethod:uint8_t{
    Lbg,
    Smo,
    NlFlux
};

enum class [[nodiscard]] HfiMethod:uint8_t{
    
};


//不做温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_25C = 0;

//使用MCU结温进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_DIEJUNC = 1;

//使用外部温度输入1(通常为FET温度)进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_EXT1 = 2;

//使用外部温度输入2(通常为转子温度)进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_EXT2 = 3;



struct FnSwitches{
    using Self = FnSwitches;

    uint32_t initial_dc_calibrate_en : 1;
    uint32_t initial_encoder_dirtest_en : 1;
    uint32_t initial_tone_music : 1;
    uint32_t res_temp_compensate_source : 2;
    uint32_t deadtime_compensate_en : 1;
    uint32_t cross_decoupling_en : 1;
    uint32_t bemf_decoupling_en : 1;

    uint32_t hfi_en : 1;
    uint32_t flux_observer_en : 1;

    uint32_t fet_temperature_sensor_equipped : 1;
    uint32_t rotor_temperature_sensor_equipped : 1;
    uint32_t busbar_voltage_sensor_equipped : 1;
    uint32_t busbar_current_sensor_equipped : 1;

    constexpr void reset(){
        *this = std::bit_cast<Self>(uint32_t(0));
    }
};


struct OpFlags{
    using Self = OpFlags;


    uint32_t initial_dc_calibrate:1;
    uint32_t initial_encoder_dirtest:1;
    uint32_t measure_resind:1;
    uint32_t measure_flux:1;
    uint32_t encoder_calibrate:1;

    constexpr void reset(){
        *this = std::bit_cast<Self>(uint32_t(0));
    }

    constexpr bool any() const {
        return std::bit_cast<uint32_t>(*this) != 0;
    }
};



struct alignas(4) [[nodiscard]] DcCalibrateState{
    std::array<uint32_t, 3> uvw_current_bits_offset_acc;
    std::array<uint16_t, 3> uvw_current_bits_offset;

    bool dc_cal_done;
    size_t dc_cal_cnt;

    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};

struct alignas(4) [[nodiscard]] AllState{
    DcCalibrateState dc_calibrate_state;

    SecondOrderState<iq16> track_ref;
    SecondOrderState<iq16> rotor_rotation_state_var;

    uq32 encoder_lap_turns;
    iiq32 encoder_multilap_turns;

    Angular<uq16> hfi_lap_angle;
    Angular<iq16> hfi_multilap_angle;

    Angular<uq32> sensed_elec_angle;
    Angular<uq32> hfi_elec_angle;
    Angular<uq32> openloop_elec_angle;
    Angular<uq32> selected_elec_angle;


    UvwCoord<iq20> uvw_curr_raw;
    
    DqCoord<iq20> dq_curr_raw;
    DqCoord<iq20> dq_curr_fastlp;

    AlphaBetaCoord<iq20> alphabeta_curr_raw;
    AlphaBetaCoord<iq20> alphabeta_curr_fastlp;
    AlphaBetaCoord<iq20> spinhfi_alphabeta_volt_gen;

    DqCoord<iq20> dq_volt_gen;

    AlphaBetaCoord<iq20> hfi_alphabeta_volt;
    AlphaBetaCoord<iq20> alphabeta_volt_gen;
    UvwCoord<iq16> uvw_dutycycle_gen;
    AlphaBetaCoord<iq20> deadtime_comp_alphabeta_dutycycle;


    iq20 busbar_curr_raw;
    iq20 busbar_curr;
    iq20 torque_curr_cmd;



    iq20 hfi_response_real_bin0;
    iq20 hfi_response_real_bin1;
    iq20 hfi_response_imag_bin1;
    iq20 hfi_response_real_bin2;
    iq20 hfi_response_imag_bin2;

    Microseconds exe_elapsed_us;
    Microseconds last_exe_begin_us;
    Microseconds exe_duration;


    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};

static constexpr uint32_t CHOPPER_FREQ = 32_KHz;
// static constexpr uint32_t CHOPPER_FREQ = 25_KHz;
// static constexpr uint32_t CHOPPER_FREQ = 10_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

static constexpr auto BUS_VOLT = iq16(12.0);
static constexpr auto INV_BUS_VOLT = 1 / BUS_VOLT;


static constexpr double SHUNT_RESISTANCE_OHMS = 0.006f;
static constexpr double OPA_GAIN = 20;
static constexpr double CURRENT_FULLSCALE_AMPS = 3.3 / (OPA_GAIN * SHUNT_RESISTANCE_OHMS);

// static constexpr auto CURRENT_AMPS_PER_ADC_LSB = uq32(CURRENT_FULLSCALE_AMPS / (1 << 12));
static constexpr auto CURRENT_AMPS_PER_ADC_LSB = iq20(CURRENT_FULLSCALE_AMPS / (1 << 12));

static constexpr size_t HFI_FREQ = 1000;

using Leso = ymd::dsp::adrc::MotorLeso;

struct MotorProfile_Gim6010{
    //伺泰威关节电机
    static constexpr size_t POLE_PAIRS = 10u;

    //100uh
    static constexpr auto PHASE_INDUCTANCE = iq20(22.3 * 1E-6);

    //1ohm
    static constexpr auto PHASE_RESISTANCE = 0.123_iq20;
};

struct MotorProfile_Ysc{
    //云深处关节电机
    static constexpr size_t POLE_PAIRS = 7u;
    static constexpr auto PHASE_INDUCTANCE = iq20(180 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.303_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.145_uq16);

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);
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
    static constexpr size_t POLE_PAIRS = 10u;
    static constexpr auto PHASE_INDUCTANCE = iq20(42 * 1E-6);
    static constexpr auto FLUX_LINKAGE = iq20(8.4 * 1E-4);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.103_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.145_uq16);

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 1000;
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);

    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_Gim4010{
    //伺泰威关节电机
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(300 * 1E-6);
    static constexpr auto PHASE_RESISTANCE = 1.03_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 1600;
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;
    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_36BLDB{
    //苏州凯航电机
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::Interior;
    static constexpr size_t POLE_PAIRS = 4u;
    static constexpr auto PHASE_INDUCTANCE = iq20(200 * 1E-6);
    static constexpr auto PHASE_RESISTANCE = 2.57_iq20;
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-6);
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.265_uq32);
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 600;
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
    static constexpr auto PHASE_RESISTANCE = 2.57_iq20;
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-6);
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 500;
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;


    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_2207{
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(19 * 1E-6);
    static constexpr auto PHASE_RESISTANCE = 0.112_iq20;
    static constexpr auto FLUX_LINKAGE = iq20(3.4 * 1E-4);
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 500;
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
    static constexpr auto PHASE_RESISTANCE = 2.45_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(4.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 400;
    static constexpr iq16 MACHINE_KP = 2.23_iq16;
    // const iq16 MACHINE_KD = 0.16_iq16;
    static constexpr iq16 MACHINE_KD = 0.045_iq16;
    using MagEncoder = drivers::VCE2755;
};

struct MotorProfile_Wheel{
    //!不具有任何凸极性
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq32>::from_turns(0.145_uq16);
    static constexpr auto MAGNETIC_STRUCTURE = MagneticStructure::SurfaceMounted;
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(86.24 * 1E-6);
    static constexpr auto FLUX_LINKAGE = iq20(1 * 1E-6);
    static constexpr auto PHASE_RESISTANCE = 0.0645_iq20;
    static constexpr auto MODU_VOLT_LIMIT = iq16(3.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 300;
    using MagEncoder = drivers::VCE2755;
};


}