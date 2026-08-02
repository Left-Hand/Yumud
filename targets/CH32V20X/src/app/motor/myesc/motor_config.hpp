#pragma once

#include "motor_profiles.hpp"
#include <cstdint>


namespace ymd::myesc{


//FOC频率
// static constexpr uint32_t CONF_FOC_FREQ = 40_KHz;
static constexpr uint32_t CONF_FOC_FREQ = 25000;
// static constexpr uint32_t CONF_FOC_FREQ = 12000;
// static constexpr uint32_t CONF_FOC_FREQ = 10_KHz;

//死区时间
static constexpr auto CONF_DEADTIME_NANOS = 120ns;
// static constexpr auto CONF_DEADTIME_NANOS = 2720ns;

//母线电压
static constexpr auto CONF_BUSBAR_VOLTAGE_F = 24.0f;


//分流电阻阻值
static constexpr double CONF_SHUNT_RESISTANCE_OHMS_F = 0.004f;

//运放增益
static constexpr double CONF_OPA_GAIN_F = 40;

//adc位数
static constexpr size_t CONF_ADC_RESOLUTION_BITS = 12;

//直流校准次数
static constexpr size_t CONF_LG2_DC_CAL_TIMES = 5 + 7;

//无感观测器pll截止频率
static constexpr size_t CONF_OBSERVER_PLL_FC = 65;

//高频注入pll截止频率
static constexpr size_t CONF_HFI_PLL_FC = 65;

//最大允许的相采样ADC值和中点的偏移，否则报错adc中点偏差过大
static constexpr uint32_t CONF_ADC_MIDPOINT_OFFSET_BVALUE_TOLERANCE = 100;

//高频注入最大调制深度
static constexpr double CONF_HFI_MODU_DEPTH_LIMIT_F = 0.06f;

//电流环最大调制深度
// static constexpr double CONF_CTRL_MODU_DEPTH_LIMIT_F = 0.39f;
static constexpr double CONF_CTRL_MODU_DEPTH_LIMIT_F = 0.39f;

//adc外设频率
static constexpr uint32_t CONF_ADC_PERIPH_FREQ = 144000000;

//adc分频比
static constexpr uint32_t CONF_NUM_ADC_CLOCK_DIVIDER = 8;

//adc完整能测得的电压
static constexpr double CONF_ADC_FULLSCALE_VOLTAGE_F = 3.3f;

#if 0
static constexpr int32_t CONF_CURVE_X2_LIMIT = 3;
static constexpr auto CONF_CURVE_X3_LIMIT = 14.5_iq16;
#endif

#if 1
static constexpr int32_t CONF_CURVE_X2_LIMIT = 8;
// static constexpr auto CONF_CURVE_X3_LIMIT = 24.5_iq16;
static constexpr auto CONF_CURVE_X3_LIMIT = 94.5_iq16;
#endif

// 原点对应的编码器单圈绝对计数值(如果使用编码器查找原点)
static constexpr uq32 CONF_HOME_ABS_OFFSET = uq32(0.7);

// 相对原点的最小位置限位
static constexpr auto CONF_CONSTRAIN_MIN_RELHOME_POSITION = iiq32(-20.2);

// 相对原点的最大位置限位
static constexpr auto CONF_CONSTRAIN_MAX_RELHOME_POSITION = iiq32(20.2);

//should below 1/sqrt(3):
// 1/sqrt(3) * 1.5 = 2 / sqrt(3)
// reach svm max duty
static_assert(CONF_CTRL_MODU_DEPTH_LIMIT_F + CONF_HFI_MODU_DEPTH_LIMIT_F < (1.0 / 1.74));

static constexpr auto BUSBAR_VOLT = iq20(CONF_BUSBAR_VOLTAGE_F);
static constexpr auto INV_BUSBAR_VOLT = uq32(1.0 / CONF_BUSBAR_VOLTAGE_F);

static constexpr iq20 CTRL_MODU_DEPTH_LIMIT = iq20(CONF_CTRL_MODU_DEPTH_LIMIT_F);
static constexpr iq20 CONF_HFI_MODU_DEPTH_LIMIT = iq20(CONF_HFI_MODU_DEPTH_LIMIT_F);
static constexpr auto CTRL_VOLT_LIMIT_F = CONF_BUSBAR_VOLTAGE_F * CONF_CTRL_MODU_DEPTH_LIMIT_F;
static constexpr iq20 CTRL_VOLT_LIMIT = iq20(CTRL_VOLT_LIMIT_F);
static constexpr uq32 INV_CTRL_VOLT_LIMIT = uq32(1.0 / CTRL_VOLT_LIMIT_F);


// #region OPA 

static constexpr double ADC_HALFSCALE_VOLTAGE_F = CONF_ADC_FULLSCALE_VOLTAGE_F / 2.0;
static constexpr double CURRENT_HALFSCALE_AMPS_F = ADC_HALFSCALE_VOLTAGE_F / (CONF_OPA_GAIN_F * CONF_SHUNT_RESISTANCE_OHMS_F);
// #endregion


static constexpr uint32_t ADC_MAXIMUM_BVALUE = (1u << CONF_ADC_RESOLUTION_BITS) - 1;
static constexpr uint32_t ADC_MIDPOINT_BVALUE = (1u << (CONF_ADC_RESOLUTION_BITS)) / 2;


static constexpr double CURRENT_AMPS_PER_ADC_LSB_F = 2 * CURRENT_HALFSCALE_AMPS_F / (1 << CONF_ADC_RESOLUTION_BITS);
static constexpr auto CURRENT_HALFSCALE_AMPS = iq20(CURRENT_HALFSCALE_AMPS_F);
static constexpr auto CURRENT_AMPS_PER_ADC_LSB = iq20(CURRENT_AMPS_PER_ADC_LSB_F);
static constexpr auto CURRENT_NOISE_STDVAR = CURRENT_AMPS_PER_ADC_LSB * 8;
static constexpr auto ADC_LSB_PER_CURRENT_AMPS = iq16(1.0f / CURRENT_AMPS_PER_ADC_LSB_F);


// static constexpr iq20 HFI_VOLT = CONF_HFI_MODU_DEPTH_LIMIT * CONF_BUSBAR_VOLT;
static constexpr double HALFWAVE_MICROS = 1000000.0 / (CONF_FOC_FREQ * 2);
static constexpr size_t TIMER_ARR_VALUE = 144000000 / (CONF_FOC_FREQ * 2) - 1;
static constexpr double ADC_SAMPLE_TICKS = (13.5 + 1.5) * 3;

static constexpr double ADC_SAMPLE_ELAPSED_MICROS = ADC_SAMPLE_TICKS * (1000000.0 / (CONF_ADC_PERIPH_FREQ / CONF_NUM_ADC_CLOCK_DIVIDER));
static constexpr double ADC_SAMPLE_TRIM_DUTYCYCLE = ADC_SAMPLE_ELAPSED_MICROS / HALFWAVE_MICROS;

static constexpr double PWMGEN_MAX_DUTYCYCLE = 1.0f - ADC_SAMPLE_TRIM_DUTYCYCLE;
static constexpr size_t ADC_SAMPLE_TRIM_CC_VALUE = (TIMER_ARR_VALUE + 1) * ADC_SAMPLE_TRIM_DUTYCYCLE;


static constexpr size_t DC_CAL_TIMES = 1 << CONF_LG2_DC_CAL_TIMES;





// using MotorProfile = MotorProfile_Ysc;
// using MotorProfile = MotorProfile_Gim4310;
using MotorProfile = MotorProfile_Jc4310;

// using MotorProfile = MotorProfile_M06Bare;
// using MotorProfile = MotorProfile_Wheel;
// using MotorProfile = MotorProfile_3505;
// using MotorProfile = MotorProfile_E800;
// using MotorProfile = MotorProfile_Gkf2023;
// using MotorProfile = MotorProfile_NidecFan;
// using MotorProfile = MotorProfile_36BLDB;
// using MotorProfile = MotorProfile_NiuLiu;
// using MotorProfile = MotorProfile_2207;



static consteval uq32 calc_uq32_rcp(const auto x){
    const uint32_t bits = uint32_t((1.0f / x) * (1ull << 32));
    return  uq32::from_bits(bits + 1);
}


static constexpr auto PHASE_RESISTANCE_OHM = MotorProfile::PHASE_RESISTANCE_OHM;
static constexpr auto Q_AXIS_INDUCTANCE_MH = MotorProfile::Q_AXIS_INDUCTANCE_MH;
static constexpr auto D_AXIS_INDUCTANCE_MH = MotorProfile::D_AXIS_INDUCTANCE_MH;
static constexpr auto PHASE_INDUCTANCE_MH = (Q_AXIS_INDUCTANCE_MH + D_AXIS_INDUCTANCE_MH) >> 1;
static constexpr auto PREFERD_CURRENT_CUTOFF_FREQ = MotorProfile::PREFERD_CURRENT_CUTOFF_FREQ;

static constexpr auto FLUX_LINKAGE = MotorProfile::FLUX_LINKAGE;
static constexpr auto POLE_PAIRS = MotorProfile::POLE_PAIRS;
static constexpr uq32 INV_POLE_PAIRS = calc_uq32_rcp(POLE_PAIRS); 


static constexpr uq32 TSAMPLE = calc_uq32_rcp(CONF_FOC_FREQ);



static constexpr int32_t E1_LIMIT = 100;
static constexpr int32_t E2_LIMIT = 1000;

static constexpr auto TORQUE_CURR_STEP_LIMIT = iq20(0.04);
static constexpr auto TORQUE_CURR_LIMIT = MotorProfile::CURRENT_LIMIT;

static constexpr iq20 HW_TORQUE_CURRENT_LIMIT = 
    std::min(
        CURRENT_HALFSCALE_AMPS, 
        iq20(CONF_BUSBAR_VOLTAGE_F * 0.666) / PHASE_RESISTANCE_OHM
    ) * uq32(0.8);
}
