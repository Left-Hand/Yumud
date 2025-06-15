#pragma once

#include "core/math/real.hpp"

namespace ymd{



// static constexpr size_t CHOP_FREQ = 30_KHz;
static constexpr size_t CHOP_FREQ = 20_KHz;
static constexpr size_t ISR_FREQ = 20_KHz * 2;
// static constexpr size_t CHOP_FREQ = 100;

static constexpr size_t STEPPER_SECTORS_PER_ROTATION = 4;
static constexpr size_t BLDC_SECTORS_PER_ROTATION = 6;

enum class MotorType:uint8_t{
    Bldc,
    Stepper
};

struct Nema42MotorSettings{
    static constexpr MotorType MOTOR_TYPE = MotorType::Stepper;
    static constexpr size_t MOTOR_POLE_PAIRS = 50;
    static constexpr size_t SECTORS_PER_ROTATION = STEPPER_SECTORS_PER_ROTATION;
    static constexpr size_t SECTORS_PER_POSITION = MOTOR_POLE_PAIRS * SECTORS_PER_ROTATION;
};

using MyMotorSettings = Nema42MotorSettings;
static constexpr size_t SECTORS_PER_ROTATION = MyMotorSettings::SECTORS_PER_ROTATION;
static constexpr size_t SECTORS_PER_POSITION = MyMotorSettings::SECTORS_PER_POSITION;
static constexpr size_t MOTOR_POLE_PAIRS = MyMotorSettings::MOTOR_POLE_PAIRS;

}