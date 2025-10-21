#pragma once

#include "mt6701_prelude.hpp"

namespace ymd::drivers{

class MT6701 final:
    public MagEncoderIntf,
    public MT6701_Regs
{
public:
    explicit MT6701(
        Some<hal::I2c *> i2c, 
        hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(MT6701_Phy(i2c, addr)){;}

    explicit MT6701(MT6701_Phy && phy):
        phy_(std::move(phy)){;}

    ~MT6701(){};

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Angle<q31>> read_lap_angle();
    
    [[nodiscard]] IResult<MagStatus> get_mag_status();

    [[nodiscard]] IResult<> enable_uvwmux(const Enable en);

    [[nodiscard]] IResult<> enable_abzmux(const Enable en);

    [[nodiscard]] IResult<> set_direction(const bool clockwise);

    [[nodiscard]] IResult<> set_poles(const uint8_t poles);

    [[nodiscard]] IResult<> set_abz_resolution(const uint16_t abz_resolution);

    [[nodiscard]] IResult<> set_zero_position(const uint16_t zero_position);

    [[nodiscard]] IResult<> set_zero_pulse_width(const ZeroPulseWidth zero_pulse_width);

    [[nodiscard]] IResult<> set_hysteresis(const Hysteresis hysteresis);

    [[nodiscard]] IResult<> enable_fast_mode(const Enable en);

    [[nodiscard]] IResult<> enable_pwm(const Enable en);

    [[nodiscard]] IResult<> set_pwm_polarity(const bool polarity);

    [[nodiscard]] IResult<> set_pwm_freq(const PwmFreq pwm_freq);

    [[nodiscard]] IResult<> set_start_position(const real_t start);

    [[nodiscard]] IResult<> set_stop_position(const real_t stop);
private:

    MT6701_Phy phy_;
    Packet packet_ = {0, 0};
    real_t lap_position_ = real_t(0);
    bool fast_mode_ = true;

};

}
