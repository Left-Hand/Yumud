#pragma once

#include "motor_profiles.hpp"


namespace ymd::myesc{


static constexpr uint32_t CHOPPER_FREQ = 36_KHz;
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
static constexpr auto CURRENT_NOISE_STDVAR = CURRENT_AMPS_PER_ADC_LSB * 8;

[[maybe_unused]] static constexpr iq20 HFI_VOLT = 1.0_iq20;


// using MotorProfile = MotorProfile_Ysc;
// using MotorProfile = MotorProfile_Gim4010;
// using MotorProfile = MotorProfile_M06Bare;
// using MotorProfile = MotorProfile_Wheel;
// using MotorProfile = MotorProfile_3505;
using MotorProfile = MotorProfile_E800;
// using MotorProfile = MotorProfile_NidecFan;
// using MotorProfile = MotorProfile_36BLDB;
// using MotorProfile = MotorProfile_NiuLiu;
// using MotorProfile = MotorProfile_2207;


}