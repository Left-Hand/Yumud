#pragma once

#include "motor_profiles.hpp"


namespace ymd::myesc{


static constexpr uint32_t CHOPPER_FREQ = 40_KHz;
// static constexpr uint32_t CHOPPER_FREQ = 25_KHz;
// static constexpr uint32_t CHOPPER_FREQ = 10_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;


// #region VOLTAGE
static constexpr auto BUS_VOLT = iq16(12.0);
static constexpr auto INV_BUS_VOLT = 1 / BUS_VOLT;


//should below 1/sqrt(3):
// 1/sqrt(3) * 1.5 = 2 / sqrt(3)
// reach svm max duty

static constexpr iq20 HFI_MODU_DEPTH_LIMIT = 0.08_iq20;
static constexpr iq20 CTRL_MODU_DEPTH_LIMIT = 0.38_iq20;

static constexpr iq20 CTRL_VOLT_LIMIT = BUS_VOLT * CTRL_MODU_DEPTH_LIMIT;
static constexpr iq20 INV_CTRL_VOLT_LIMIT = 1 / CTRL_VOLT_LIMIT;
// #endregion

// #region OPA 
static constexpr double SHUNT_RESISTANCE_OHMS = 0.006f;
static constexpr double OPA_GAIN = 20;
static constexpr double CURRENT_FULLSCALE_AMPS = 3.3 / (OPA_GAIN * SHUNT_RESISTANCE_OHMS);
// #endregion

static constexpr auto DEADTIME_NANOS = 120ns;
// static constexpr auto DEADTIME_NANOS = 920ns;

static constexpr size_t PLL_FC = 75;
static constexpr auto PLL_ZETA = 2.0_iq16;



// static constexpr auto CURRENT_AMPS_PER_ADC_LSB = uq32(CURRENT_FULLSCALE_AMPS / (1 << 12));
static constexpr auto CURRENT_AMPS_PER_ADC_LSB = iq20(CURRENT_FULLSCALE_AMPS / (1 << 12));
static constexpr auto CURRENT_NOISE_STDVAR = CURRENT_AMPS_PER_ADC_LSB * 8;


static constexpr iq20 HFI_VOLT = HFI_MODU_DEPTH_LIMIT * BUS_VOLT;
static constexpr float HALFWAVE_MICROS = 1000000.0 / (CHOPPER_FREQ * 2);
static constexpr size_t TIMER_ARR_VALUE = 144000000 / (CHOPPER_FREQ * 2) - 1;
static constexpr float ADC_SAMPLE_TICKS = (13.5 + 1.5) * 3;
static constexpr float ADC_ALIGNED_IPCORE_FREQ = 144000000;
static constexpr float ADC_CLOCK_DIVIDER_COUNT = 8;
static constexpr float ADC_SAMPLE_ELAPSED_MICROS = ADC_SAMPLE_TICKS * (1000000.0 / (ADC_ALIGNED_IPCORE_FREQ / ADC_CLOCK_DIVIDER_COUNT));
static constexpr float ADC_SAMPLE_TRIM_DUTYCYCLE = ADC_SAMPLE_ELAPSED_MICROS / HALFWAVE_MICROS;

static constexpr float PWMGEN_MAX_DUTYCYCLE = 1.0f - ADC_SAMPLE_TRIM_DUTYCYCLE;
static constexpr size_t ADC_SAMPLE_TRIM_CC_VALUE = (TIMER_ARR_VALUE + 1) * ADC_SAMPLE_TRIM_DUTYCYCLE;

static constexpr size_t DC_CAL_TIMES = 32 * 128;


// using MotorProfile = MotorProfile_Ysc;
// using MotorProfile = MotorProfile_Gim4010;
// using MotorProfile = MotorProfile_M06Bare;
// using MotorProfile = MotorProfile_Wheel;
// using MotorProfile = MotorProfile_3505;
// using MotorProfile = MotorProfile_E800;
// using MotorProfile = MotorProfile_Gkf2023;
using MotorProfile = MotorProfile_NidecFan;
// using MotorProfile = MotorProfile_36BLDB;
// using MotorProfile = MotorProfile_NiuLiu;
// using MotorProfile = MotorProfile_2207;


}