#pragma once

#include "motor_profiles.hpp"


namespace ymd::myesc{


// static constexpr uint32_t CHOPPER_FREQ = 40_KHz;
static constexpr uint32_t CHOPPER_FREQ = 25_KHz;
// static constexpr uint32_t CHOPPER_FREQ = 10_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

static constexpr auto DEADTIME_NANOS = 120ns;
// static constexpr auto DEADTIME_NANOS = 2720ns;



// #region VOLTAGE
static constexpr auto BUSBAR_VOLT = iq20(24.0);
static constexpr auto INV_BUSBAR_VOLT = uq32(1 / BUSBAR_VOLT);


//should below 1/sqrt(3):
// 1/sqrt(3) * 1.5 = 2 / sqrt(3)
// reach svm max duty

static constexpr iq20 HFI_MODU_DEPTH_LIMIT = 0.06_iq20;
static constexpr iq20 CTRL_MODU_DEPTH_LIMIT = 0.39_iq20;

static constexpr iq20 CTRL_VOLT_LIMIT = BUSBAR_VOLT * CTRL_MODU_DEPTH_LIMIT;
static constexpr uq32 INV_CTRL_VOLT_LIMIT = uq32(1 / CTRL_VOLT_LIMIT);
// #endregion

// #region OPA 
static constexpr double SHUNT_RESISTANCE_OHMS = 0.004f;
static constexpr double HW_OPA_GAIN = 20;
static constexpr double ONCHIP_ADC_OPA_GAIN = 1;
static constexpr double OPA_GAIN = HW_OPA_GAIN * ONCHIP_ADC_OPA_GAIN;
static constexpr double CURRENT_HALFSCALE_AMPS_F = 1.65 / (OPA_GAIN * SHUNT_RESISTANCE_OHMS);
// #endregion




// static constexpr auto CURRENT_AMPS_PER_ADC_LSB = uq32(CURRENT_HALFSCALE_AMPS_F / (1 << 12));

static constexpr double CURRENT_AMPS_PER_ADC_LSB_F = 2 * CURRENT_HALFSCALE_AMPS_F / (1 << 12);
static constexpr auto CURRENT_HALFSCALE_AMPS = iq20(CURRENT_HALFSCALE_AMPS_F);
static constexpr auto CURRENT_AMPS_PER_ADC_LSB = iq20(CURRENT_AMPS_PER_ADC_LSB_F);
static constexpr auto CURRENT_NOISE_STDVAR = CURRENT_AMPS_PER_ADC_LSB * 8;


// static constexpr iq20 HFI_VOLT = HFI_MODU_DEPTH_LIMIT * BUSBAR_VOLT;
static constexpr float HALFWAVE_MICROS = 1000000.0 / (CHOPPER_FREQ * 2);
static constexpr size_t TIMER_ARR_VALUE = 144000000 / (CHOPPER_FREQ * 2) - 1;
static constexpr float ADC_SAMPLE_TICKS = (13.5 + 1.5) * 3;
static constexpr float ADC_ALIGNED_IPCORE_FREQ = 144000000;
static constexpr float ADC_CLOCK_DIVIDER_COUNT = 8;
static constexpr float ADC_SAMPLE_ELAPSED_MICROS = ADC_SAMPLE_TICKS * (1000000.0 / (ADC_ALIGNED_IPCORE_FREQ / ADC_CLOCK_DIVIDER_COUNT));
static constexpr float ADC_SAMPLE_TRIM_DUTYCYCLE = ADC_SAMPLE_ELAPSED_MICROS / HALFWAVE_MICROS;

static constexpr float PWMGEN_MAX_DUTYCYCLE = 1.0f - ADC_SAMPLE_TRIM_DUTYCYCLE;
static constexpr size_t ADC_SAMPLE_TRIM_CC_VALUE = (TIMER_ARR_VALUE + 1) * ADC_SAMPLE_TRIM_DUTYCYCLE;

static constexpr size_t LG2_DC_CAL_TIMES = 5 + 7;
static constexpr size_t DC_CAL_TIMES = 1 << LG2_DC_CAL_TIMES;


// using MotorProfile = MotorProfile_Ysc;
using MotorProfile = MotorProfile_Gim4310;

// using MotorProfile = MotorProfile_M06Bare;
// using MotorProfile = MotorProfile_Wheel;
// using MotorProfile = MotorProfile_3505;
// using MotorProfile = MotorProfile_E800;
// using MotorProfile = MotorProfile_Gkf2023;
// using MotorProfile = MotorProfile_NidecFan;
// using MotorProfile = MotorProfile_36BLDB;
// using MotorProfile = MotorProfile_NiuLiu;
// using MotorProfile = MotorProfile_2207;


}